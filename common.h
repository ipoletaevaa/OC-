#ifndef COMMON_H
#define COMMON_H

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <time.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFER_SIZE 256
#define SHM_NAME_PREFIX "/lab3_shm"
#define SEM_NAME_PREFIX "/lab3_sem"

typedef struct {
    char data[BUFFER_SIZE];
    int data_ready;
    int for_child1;
    int stop_signal;
} shared_data_t;

void remove_vowels(char *str);
void generate_unique_name(char *buffer, const char *prefix);

#endif