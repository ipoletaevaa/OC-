#include "common.h"

void remove_vowels(char *str) {
    char result[BUFFER_SIZE];
    int j = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        char c = str[i];
        if (!(c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' ||
              c == 'y' || c == 'A' || c == 'E' || c == 'I' || c == 'O' ||
              c == 'U' || c == 'Y')) {
            result[j++] = c;
        }
    }
    result[j] = '\0';
    strcpy(str, result);
}

int main(int argc, char **argv) {
    if (argc != 4) {
        const char msg[] = "Usage: child1 <shm_name> <sem_name> <filename>\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    const char *shm_name = argv[1];
    const char *sem_name = argv[2];
    const char *filename = argv[3];

    int shm_fd = shm_open(shm_name, O_RDWR, 0666);
    if (shm_fd == -1) {
        const char msg[] = "Child1: error opening shared memory\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    shared_data_t *shared_data = mmap(NULL, sizeof(shared_data_t), 
                                     PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_data == MAP_FAILED) {
        const char msg[] = "Child1: error mapping shared memory\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    sem_t *semaphore = sem_open(sem_name, 0);
    if (semaphore == SEM_FAILED) {
        const char msg[] = "Child1: error opening semaphore\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    int file = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (file == -1) {
        const char msg[] = "Child1: error opening file\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    const char start_msg[] = "Child1 started and waiting for data...\n";
    write(STDOUT_FILENO, start_msg, sizeof(start_msg) - 1);

    while (1) {
        sem_wait(semaphore);
        
        if (shared_data->stop_signal) {
            sem_post(semaphore);
            break;
        }
        
        if (shared_data->data_ready && shared_data->for_child1) {
            char processed[BUFFER_SIZE];
            strcpy(processed, shared_data->data);
            shared_data->data_ready = 0;
            sem_post(semaphore);
            
            remove_vowels(processed);
            write(file, processed, strlen(processed));
            write(file, "\n", 1);
            write(STDOUT_FILENO, "Child1 processed: ", 18);
            write(STDOUT_FILENO, processed, strlen(processed));
            write(STDOUT_FILENO, "\n", 1);
        } else {
            sem_post(semaphore);
        }
        
        sleep(1);
    }

    close(file);
    munmap(shared_data, sizeof(shared_data_t));
    close(shm_fd);
    sem_close(semaphore);

    const char finish_msg[] = "Child1 finished.\n";
    write(STDOUT_FILENO, finish_msg, sizeof(finish_msg) - 1);
    return 0;
}