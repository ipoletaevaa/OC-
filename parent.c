#include "common.h"

void generate_unique_name(char *buffer, const char *prefix) {
    pid_t pid = getpid();
    time_t now = time(NULL);
    
    int i = 0;
    const char *p = prefix;
    while (*p && i < BUFFER_SIZE - 1) {
        buffer[i++] = *p++;
    }
    
    buffer[i++] = '_';
    int pid_temp = pid;
    char pid_str[20];
    char *pid_ptr = pid_str + 19;
    *pid_ptr = '\0';
    do {
        *--pid_ptr = '0' + (pid_temp % 10);
        pid_temp /= 10;
    } while (pid_temp > 0);
    
    while (*pid_ptr && i < BUFFER_SIZE - 1) {
        buffer[i++] = *pid_ptr++;
    }
    
    buffer[i++] = '_';
    time_t time_temp = now;
    char time_str[20];
    char *time_ptr = time_str + 19;
    *time_ptr = '\0';
    do {
        *--time_ptr = '0' + (time_temp % 10);
        time_temp /= 10;
    } while (time_temp > 0);
    
    while (*time_ptr && i < BUFFER_SIZE - 1) {
        buffer[i++] = *time_ptr++;
    }
    
    buffer[i] = '\0';
}

int main() {
    char shm_name[BUFFER_SIZE];
    char sem_name[BUFFER_SIZE];
    generate_unique_name(shm_name, SHM_NAME_PREFIX);
    generate_unique_name(sem_name, SEM_NAME_PREFIX);

    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        const char msg[] = "Error creating shared memory\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    char zero_byte = 0;
    for (size_t i = 0; i < sizeof(shared_data_t); i++) {
        if (write(shm_fd, &zero_byte, 1) != 1) {
            const char msg[] = "Error setting shared memory size\n";
            write(STDERR_FILENO, msg, sizeof(msg) - 1);
            exit(EXIT_FAILURE);
        }
    }
    lseek(shm_fd, 0, SEEK_SET);

    shared_data_t *shared_data = mmap(NULL, sizeof(shared_data_t), 
                                     PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_data == MAP_FAILED) {
        const char msg[] = "Error mapping shared memory\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    memset(shared_data, 0, sizeof(shared_data_t));
    shared_data->data_ready = 0;
    shared_data->stop_signal = 0;

    sem_t *semaphore = sem_open(sem_name, O_CREAT, 0666, 1);
    if (semaphore == SEM_FAILED) {
        const char msg[] = "Error creating semaphore\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    char filename1[BUFFER_SIZE];
    char filename2[BUFFER_SIZE];
    int bytes_read;

    write(STDOUT_FILENO, "Enter filename for child1: ", 27);
    bytes_read = read(STDIN_FILENO, filename1, sizeof(filename1) - 1);
    filename1[bytes_read - 1] = '\0';

    write(STDOUT_FILENO, "Enter filename for child2: ", 27);
    bytes_read = read(STDIN_FILENO, filename2, sizeof(filename2) - 1);
    filename2[bytes_read - 1] = '\0';

    pid_t child1 = fork();
    if (child1 == 0) {
        execl("./child1", "child1", shm_name, sem_name, filename1, NULL);
        const char msg[] = "Error starting child1\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    pid_t child2 = fork();
    if (child2 == 0) {
        execl("./child2", "child2", shm_name, sem_name, filename2, NULL);
        const char msg[] = "Error starting child2\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    sleep(1);

    write(STDOUT_FILENO, "Enter strings (empty line to finish):\n", 39);

    srand(time(NULL));
    char buffer[BUFFER_SIZE];

    while ((bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer) - 1)) > 0) {
        if (bytes_read == 1 && buffer[0] == '\n') {
            break;
        }

        buffer[bytes_read - 1] = '\0';

        int for_child1 = (rand() % 100 < 80) ? 1 : 0;

        sem_wait(semaphore);
        strcpy(shared_data->data, buffer);
        shared_data->data_ready = 1;
        shared_data->for_child1 = for_child1;
        sem_post(semaphore);

        sleep(1);
    }

    sem_wait(semaphore);
    shared_data->stop_signal = 1;
    sem_post(semaphore);

    wait(NULL);
    wait(NULL);

    munmap(shared_data, sizeof(shared_data_t));
    close(shm_fd);
    shm_unlink(shm_name);
    sem_close(semaphore);
    sem_unlink(sem_name);

    const char finish_msg[] = "Parent process finished.\n";
    write(STDOUT_FILENO, finish_msg, sizeof(finish_msg) - 1);
    return 0;
}