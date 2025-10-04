#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#define BUFFER_SIZE 100

void remove_vowels(char *str) {
    char result[BUFFER_SIZE];
    int j = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        char c = str[i];
        if (!(c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' ||
              c == 'y' || c == 'A' || c == 'E' || c == 'I' || c == 'O' ||
              c == 'U' || c == 'Y' ||
              c == 'а' || c == 'е' || c == 'ё' || c == 'и' || c == 'о' ||
              c == 'у' || c == 'ы' || c == 'э' || c == 'ю' || c == 'я' ||
              c == 'А' || c == 'Е' || c == 'Ё' || c == 'И' || c == 'О' ||
              c == 'У' || c == 'Ы' || c == 'Э' || c == 'Ю' || c == 'Я')) {
            result[j++] = c;
        }
    }
    result[j] = '\0';
    strcpy(str, result);
}

int main(int argc, char **argv) {
    char buffer[BUFFER_SIZE];
    int bytes_read;
    
    if (argc != 2) {
        const char msg[] = "Введите: child1 filename\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }

    int file = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (file == -1) {
        const char msg[] = "Ошибка открытия файла\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }

    while ((bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer) - 1)) > 0) {
        if (bytes_read == 4 && strcmp(buffer, "END") == 0) break;
        
        buffer[bytes_read] = '\0';
        remove_vowels(buffer);
        write(file, buffer, strlen(buffer));
        write(file, "\n", 1);
    }

    close(file);
    return 0;
}
