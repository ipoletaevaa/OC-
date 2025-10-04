#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>

#define BUFFER_SIZE 100

int main() {
    int pipe1[2], pipe2[2];
    pid_t child1, child2; 
    char filename1[50], filename2[50]; 
    char buffer[BUFFER_SIZE];
    int bytes_read;

    srand(time(NULL));

    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        const char msg[] = "Ошибка создания канала\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }

    write(STDOUT_FILENO, "Введите имя для child1: ", 38);
    bytes_read = read(STDIN_FILENO, filename1, sizeof(filename1) - 1);
    filename1[bytes_read - 1] = '\0';

    write(STDOUT_FILENO, "Введите имя для child2: ", 38);
    bytes_read = read(STDIN_FILENO, filename2, sizeof(filename2) - 1);
    filename2[bytes_read - 1] = '\0';

    child1 = fork();
    if (child1 == 0) {
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);
        dup2(pipe1[0], STDIN_FILENO);
        close(pipe1[0]);
        execl("./child1", "child1", filename1, NULL);
        const char msg[] = "Ошибка выполнения команды child1\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }

    child2 = fork();
    if (child2 == 0) {
        close(pipe2[1]);
        close(pipe1[0]);
        close(pipe1[1]);
        dup2(pipe2[0], STDIN_FILENO);
        close(pipe2[0]);
        execl("./child2", "child2", filename2, NULL);
        const char msg[] = "Ошибка выполнения команды child2\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }

    close(pipe1[0]);
    close(pipe2[0]);

    write(STDOUT_FILENO, "Введите строки. Для остановки пустая строка. \n", 81);

    while ((bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer) - 1)) > 0) {
        if (bytes_read == 1 && buffer[0] == '\n') break;
        
        buffer[bytes_read - 1] = '\0';

        if (rand() % 100 < 80) {
            write(pipe1[1], buffer, strlen(buffer) + 1);
        } else {
            write(pipe2[1], buffer, strlen(buffer) + 1);
        }
    }

    const char end_msg[] = "END";
    write(pipe1[1], end_msg, sizeof(end_msg));
    write(pipe2[1], end_msg, sizeof(end_msg));

    close(pipe1[1]);
    close(pipe2[1]);

    wait(NULL);
    wait(NULL);

    return 0;
}
