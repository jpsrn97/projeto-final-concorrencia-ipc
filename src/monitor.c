#include "../include/common.h"

/*
 * Monitor:
 *  - Anexa à memória compartilhada POSIX criada pelo manager (SHM_NAME).
 *  - Lê periodicamente as estatísticas globais (job_stats_t).
 *  - Exibe no terminal em "tempo quase real".
 */

static volatile sig_atomic_t keep_running = 1;

static void handle_sigint(int sig) {
    (void)sig;
    keep_running = 0;
}

int main(void) {
    printf("=== Projeto Final Concorrência & IPC: Monitor ===\n");
    printf("PID do monitor: %d\n", getpid());
    printf("Certifique-se de iniciar o 'manager' antes deste processo.\n\n");

    signal(SIGINT, handle_sigint);

    // 1) Abre a memória compartilhada já criada pelo manager
    int fd = shm_open(SHM_NAME, O_RDONLY, 0666);
    if (fd == -1) {
        perror("shm_open");
        fprintf(stderr, "Talvez o 'manager' ainda não tenha sido iniciado.\n");
        return 1;
    }

    // 2) Faz o mapeamento apenas para leitura
    void *addr = mmap(NULL,
                      sizeof(job_stats_t),
                      PROT_READ,
                      MAP_SHARED,
                      fd,
                      0);
    close(fd);

    if (addr == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    job_stats_t *stats = (job_stats_t *)addr;

    printf("Monitor conectado à shared memory '%s'.\n", SHM_NAME);
    printf("Pressione Ctrl+C para sair.\n\n");

    // 3) Loop de monitoramento
    while (keep_running) {
        printf("[MONITOR] total_criados=%d | em_fila=%d | em_proc=%d | concluidos=%d\n",
               stats->total_jobs_created,
               stats->jobs_in_queue,
               stats->jobs_in_progress,
               stats->jobs_completed);
        fflush(stdout);
        sleep(1);
    }

    printf("\nMonitor encerrado.\n");
    return 0;
}
