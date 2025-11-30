#include "../include/common.h"
#include <sys/wait.h>  // waitpid

// ------------------------ FILA DE JOBS (PRODUTOR–CONSUMIDOR) ------------------------

typedef struct {
    job_t buffer[MAX_JOBS_QUEUE];
    int head;
    int tail;
    int count;
    pthread_mutex_t mtx;
    pthread_cond_t  not_empty;
    pthread_cond_t  not_full;
} job_queue_t;

static job_queue_t g_queue;
static sem_t g_sem_capacity;         // limita quantos jobs podem estar em processamento
static job_stats_t *g_stats = NULL;  // ponteiro para memória compartilhada
static int g_pipe_read_fd = -1;
static volatile sig_atomic_t g_running = 1;


// ------------------------ FUNÇÕES DA FILA ------------------------

static void job_queue_init(job_queue_t *q) {
    q->head = 0;
    q->tail = 0;
    q->count = 0;
    pthread_mutex_init(&q->mtx, NULL);
    pthread_cond_init(&q->not_empty, NULL);
    pthread_cond_init(&q->not_full, NULL);
}

static void job_queue_push(job_queue_t *q, job_t job) {
    pthread_mutex_lock(&q->mtx);

    while (q->count == MAX_JOBS_QUEUE) {
        // fila cheia → espera espaço
        pthread_cond_wait(&q->not_full, &q->mtx);
    }

    q->buffer[q->tail] = job;
    q->tail = (q->tail + 1) % MAX_JOBS_QUEUE;
    q->count++;

    if (g_stats) {
        g_stats->jobs_in_queue++;
    }

    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->mtx);
}

static job_t job_queue_pop(job_queue_t *q) {
    pthread_mutex_lock(&q->mtx);

    while (q->count == 0 && g_running) {
        // fila vazia → espera alguém inserir
        pthread_cond_wait(&q->not_empty, &q->mtx);
    }

    job_t j = { .job_id = -1, .processing_time_ms = 0 };

    if (q->count > 0) {
        j = q->buffer[q->head];
        q->head = (q->head + 1) % MAX_JOBS_QUEUE;
        q->count--;

        if (g_stats) {
            g_stats->jobs_in_queue--;
        }

        pthread_cond_signal(&q->not_full);
    }

    pthread_mutex_unlock(&q->mtx);
    return j;
}


// ------------------------ THREAD RECEBEDORA (PIPE -> FILA) ------------------------

static void* receiver_thread(void *arg) {
    (void)arg;

    while (g_running) {
        job_t job;
        ssize_t n = read(g_pipe_read_fd, &job, sizeof(job));

        if (n == 0) {
            // pipe fechado pelo gerador → fim natural
            break;
        } else if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("[MANAGER] read(pipe)");
            break;
        } else if (n != (ssize_t)sizeof(job)) {
            fprintf(stderr, "[MANAGER] Leitura parcial de job no pipe\n");
            continue;
        }

        if (g_stats) {
            g_stats->total_jobs_created++;
        }

        printf("[MANAGER] Recebeu job %d (%d ms) do gerador\n",
               job.job_id, job.processing_time_ms);
        fflush(stdout);

        job_queue_push(&g_queue, job);
    }

    printf("[MANAGER] Receiver thread finalizando.\n");
    fflush(stdout);
    return NULL;
}


// ------------------------ THREAD WORKER (PROCESSA JOBS) ------------------------

static void* worker_thread(void *arg) {
    long id = (long)arg;

    while (g_running) {
        job_t job = job_queue_pop(&g_queue);

        if (job.job_id < 0) {
            // nada para processar
            continue;
        }

        // controla quantos jobs podem estar em execução simultaneamente
        sem_wait(&g_sem_capacity);

        if (g_stats) {
            g_stats->jobs_in_progress++;
        }

        printf("  [WORKER %ld] Iniciando job %d (%d ms)\n",
               id, job.job_id, job.processing_time_ms);
        fflush(stdout);

        // simula processamento
        usleep(job.processing_time_ms * 1000);

        printf("  [WORKER %ld] Finalizou job %d\n", id, job.job_id);
        fflush(stdout);

        if (g_stats) {
            g_stats->jobs_in_progress--;
            g_stats->jobs_completed++;
        }

        sem_post(&g_sem_capacity);
    }

    printf("  [WORKER %ld] finalizando.\n", id);
    fflush(stdout);
    return NULL;
}


// ------------------------ PROCESSO FILHO GERADOR DE JOBS ------------------------

static void run_job_generator(int pipe_write_fd) {
    srand((unsigned)time(NULL) ^ getpid());

    for (int i = 1; i <= MAX_JOBS_TO_GENERATE; i++) {
        job_t job;
        job.job_id = i;
        job.processing_time_ms = 500 + (rand() % 1500); // 500–2000 ms

        printf("[GENERATOR] Criando job %d (%d ms)\n",
               job.job_id, job.processing_time_ms);
        fflush(stdout);

        if (write(pipe_write_fd, &job, sizeof(job)) != (ssize_t)sizeof(job)) {
            perror("[GENERATOR] write(pipe)");
            break;
        }

        sleep(1); // simula chegada espaçada de jobs
    }

    printf("[GENERATOR] Fim da geração de jobs. Encerrando.\n");
    fflush(stdout);
    close(pipe_write_fd);
    _exit(0);
}


// ------------------------ SETUP DA SHARED MEMORY ------------------------

static job_stats_t* setup_shared_memory(void) {
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open");
        return NULL;
    }

    if (ftruncate(fd, sizeof(job_stats_t)) == -1) {
        perror("ftruncate");
        close(fd);
        return NULL;
    }

    void *addr = mmap(NULL,
                      sizeof(job_stats_t),
                      PROT_READ | PROT_WRITE,
                      MAP_SHARED,
                      fd,
                      0);
    close(fd);

    if (addr == MAP_FAILED) {
        perror("mmap");
        return NULL;
    }

    job_stats_t *stats = (job_stats_t *)addr;
    memset(stats, 0, sizeof(*stats)); // zera estatísticas

    return stats;
}


// ------------------------ MAIN DO PROCESSO GERENTE ------------------------

int main(void) {
    printf("=== Projeto Final Concorrência & IPC: Manager ===\n");

    // 1) Configura memória compartilhada
    g_stats = setup_shared_memory();
    if (!g_stats) {
        fprintf(stderr, "[MANAGER] Falha ao configurar memória compartilhada.\n");
        return 1;
    }

    // 2) Cria pipe entre gerente (pai) e gerador (filho)
    int fds[2];
    if (pipe(fds) == -1) {
        perror("pipe");
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // Processo filho: gerador
        close(fds[0]); // fecha leitura
        run_job_generator(fds[1]);
        // não retorna
    }

    // Processo pai: manager
    close(fds[1]);          // fecha escrita
    g_pipe_read_fd = fds[0];

    // 3) Inicializa fila, semáforo e threads
    job_queue_init(&g_queue);

    if (sem_init(&g_sem_capacity, 0, 2) != 0) {
        perror("sem_init");
        return 1;
    }

    pthread_t receiver;
    pthread_t workers[NUM_WORKERS];

    if (pthread_create(&receiver, NULL, receiver_thread, NULL) != 0) {
        perror("pthread_create(receiver)");
        return 1;
    }

    for (long i = 0; i < NUM_WORKERS; i++) {
        if (pthread_create(&workers[i], NULL, worker_thread, (void *)i) != 0) {
            perror("pthread_create(worker)");
            return 1;
        }
    }

    // 4) Aguarda o gerador terminar
    int status = 0;
    waitpid(pid, &status, 0);
    printf("[MANAGER] Processo gerador terminou.\n");
    fflush(stdout);

    // Mantém o manager vivo um tempo para processar os jobs gerados
    sleep(10);

    // Inicia encerramento
    g_running = 0;
    pthread_cond_broadcast(&g_queue.not_empty);

    pthread_join(receiver, NULL);
    for (int i = 0; i < NUM_WORKERS; i++) {
        pthread_join(workers[i], NULL);
    }

    sem_destroy(&g_sem_capacity);

    printf("[MANAGER] Encerrando.\n");
    fflush(stdout);
    return 0;
}
