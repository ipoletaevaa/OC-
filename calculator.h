#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <pthread.h>
#include <stdint.h>

#define MAX_THREADS 64
#define DEFAULT_MEMORY_LIMIT 100

typedef struct {
    char **lines;
    int count;
    __int128_t *sum;
    pthread_mutex_t *mutex;
    int *current_index;
    pthread_cond_t *cond;
    int thread_id;
    int total_threads;
} thread_data_t;

__int128_t hex_to_int128(const char *hex);
void* process_numbers(void *arg);
char** read_file_with_memory_limit(const char *filename, int memory_limit_mb, int *line_count);

#endif