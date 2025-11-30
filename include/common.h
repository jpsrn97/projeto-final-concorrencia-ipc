#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <string.h>
#include <errno.h>

// Nome da memória compartilhada POSIX usada entre manager e monitor
#define SHM_NAME "/job_stats_shm"

// Parâmetros gerais do sistema
#define MAX_JOBS_QUEUE        64   // Tamanho máximo da fila de jobs
#define NUM_WORKERS           4    // Número de threads trabalhadoras
#define MAX_JOBS_TO_GENERATE  20   // Quantos jobs o gerador cria antes de parar

// Representa um "job" a ser processado
typedef struct {
    int job_id;
    int processing_time_ms;        // tempo "simulado" de processamento (em ms)
} job_t;

// Estatísticas globais compartilhadas via memória compartilhada
typedef struct {
    int total_jobs_created;        // total de jobs criados pelo gerador
    int jobs_in_queue;             // jobs atualmente na fila interna
    int jobs_in_progress;          // jobs sendo processados neste momento
    int jobs_completed;            // jobs já concluídos
} job_stats_t;

#endif // COMMON_H
