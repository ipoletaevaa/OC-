#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "calculator.h"

void print_str(const char* str) {
    write(STDOUT_FILENO, str, strlen(str));
}

void print_error(const char* str) {
    write(STDERR_FILENO, str, strlen(str));
}

void print_int(int value) {
    char buffer[20];
    char *p = buffer + 19;
    *p = '\0';
    
    if (value == 0) {
        print_str("0");
        return;
    }
    
    int is_negative = 0;
    if (value < 0) {
        is_negative = 1;
        value = -value;
    }
    
    while (value > 0) {
        *--p = '0' + (value % 10);
        value /= 10;
    }
    
    if (is_negative) {
        *--p = '-';
    }
    
    print_str(p);
}

void print_hex(uint64_t value) {
    char buffer[17];
    buffer[16] = '\0';
    
    for (int i = 15; i >= 0; i--) {
        int nibble = (value >> (i * 4)) & 0xF;
        buffer[15 - i] = nibble < 10 ? '0' + nibble : 'A' + (nibble - 10);
    }
    
    print_str("0x");
    print_str(buffer);
}

void print_int128_hex(__int128_t value) {
    uint64_t high = (uint64_t)(value >> 64);
    uint64_t low = (uint64_t)value;
    
    if (high == 0) {
        print_hex(low);
    } else {
        print_hex(high);
        // Дополняем ведущими нулями младшую часть
        char buffer[17];
        buffer[16] = '\0';
        for (int i = 15; i >= 0; i--) {
            int nibble = (low >> (i * 4)) & 0xF;
            buffer[15 - i] = nibble < 10 ? '0' + nibble : 'A' + (nibble - 10);
        }
        print_str(buffer);
    }
}

void print_formatted_line(const char* label, int value) {
    print_str("│ ");
    print_str(label);
    print_str(": ");
    print_int(value);
    
    // Вычисляем количество пробелов для выравнивания
    int label_len = 0;
    while (label[label_len] != '\0') label_len++;
    
    int value_len = 1;
    int temp = value;
    if (temp < 0) {
        value_len++;
        temp = -temp;
    }
    while (temp >= 10) {
        value_len++;
        temp /= 10;
    }
    
    int total_len = label_len + value_len + 2; // +2 для ": "
    int spaces = 49 - total_len;
    for (int i = 0; i < spaces; i++) print_str(" ");
    print_str("│\n");
}

void print_results_table(int threads, int memory_mb, int numbers_count, __int128_t result) {
    print_str("\n┌─────────────────────────────────────────────────────────────┐\n");
    print_str("│                    РЕЗУЛЬТАТЫ ВЫЧИСЛЕНИЙ                   │\n");
    print_str("├─────────────────────────────────────────────────────────────┤\n");
    
    print_formatted_line("Потоки", threads);
    print_formatted_line("Память (МБ)", memory_mb);
    print_formatted_line("Чисел обработано", numbers_count);
    
    print_str("├─────────────────────────────────────────────────────────────┤\n");
    print_str("│ Среднее значение   : ");
    
    if (result >> 64 == 0 && (int64_t)result >= 0) {
        print_int((int)result);
        print_str("                                  │\n");
    } else {
        print_int128_hex(result);
        print_str("   │\n");
    }
    print_str("└─────────────────────────────────────────────────────────────┘\n");
}

void print_threads_info(pid_t pid, int thread_count) {
    print_str("\n┌─────────────────────────────────────────────────────────────┐\n");
    print_str("│                  ДЕМОНСТРАЦИЯ ПОТОКОВ                      │\n");
    print_str("├─────────────────────────────────────────────────────────────┤\n");
    
    print_str("│ PID процесса: ");
    print_int(pid);
    print_str("                                         │\n");
    
    print_str("│ Создано потоков: ");
    print_int(thread_count + 1);
    print_str("                                       │\n");
    
    print_str("├─────────────────────────────────────────────────────────────┤\n");
    print_str("│ Для просмотра потоков выполните в другом терминале:        │\n");
    print_str("│                                                             │\n");
    
    print_str("│ $ ps -L -p ");
    print_int(pid);
    print_str("                                            │\n");
    
    print_str("│ ИЛИ                                                        │\n");
    
    print_str("│ $ top -H -p ");
    print_int(pid);
    print_str("                                            │\n");
    
    print_str("└─────────────────────────────────────────────────────────────┘\n");
}

void print_configuration(const char* filename, int threads, int memory) {
    print_str("┌─────────────────────────────────────────────────────────────┐\n");
    print_str("│               ЗАПУСК ПАРАЛЛЕЛЬНЫХ ВЫЧИСЛЕНИЙ              │\n");
    print_str("├─────────────────────────────────────────────────────────────┤\n");
    print_str("│ Конфигурация:                                              │\n");
    
    print_str("│   Файл: ");
    print_str(filename);
    int filename_len = 0;
    while (filename[filename_len] != '\0') filename_len++;
    for (int i = filename_len; i < 50; i++) print_str(" ");
    print_str(" │\n");
    
    print_str("│   Потоки: ");
    print_int(threads);
    int threads_len = 1;
    int temp = threads;
    if (temp < 0) {
        threads_len++;
        temp = -temp;
    }
    while (temp >= 10) {
        threads_len++;
        temp /= 10;
    }
    for (int i = 0; i < 48 - threads_len; i++) print_str(" ");
    print_str(" │\n");
    
    print_str("│   Память: ");
    print_int(memory);
    int memory_len = 1;
    temp = memory;
    if (temp < 0) {
        memory_len++;
        temp = -temp;
    }
    while (temp >= 10) {
        memory_len++;
        temp /= 10;
    }
    for (int i = 0; i < 47 - memory_len; i++) print_str(" ");
    print_str(" │\n");
    
    print_str("└─────────────────────────────────────────────────────────────┘\n");
}

int main(int argc, char *argv[]) {
    int max_threads = 4;
    int memory_limit = DEFAULT_MEMORY_LIMIT;
    char *filename = "numbers.txt";
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--threads") == 0) {
            if (i + 1 < argc) max_threads = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--memory") == 0) {
            if (i + 1 < argc) memory_limit = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--file") == 0) {
            if (i + 1 < argc) filename = argv[++i];
        }
    }
    
    if (max_threads <= 0 || max_threads > MAX_THREADS) {
        print_error("Ошибка: количество потоков должно быть от 1 до ");
        print_int(MAX_THREADS);
        print_error("\n");
        return 1;
    }
    
    print_configuration(filename, max_threads, memory_limit);
    
    int line_count;
    char **lines = read_file_with_memory_limit(filename, memory_limit, &line_count);
    
    if (!lines || line_count == 0) {
        print_error("Ошибка чтения файла или файл пуст\n");
        return 1;
    }
    
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    
    __int128_t total_sum = 0;
    int current_index = 0;
    
    pthread_t threads[max_threads];
    thread_data_t thread_data[max_threads];
    
    for (int i = 0; i < max_threads; i++) {
        thread_data[i].lines = lines;
        thread_data[i].count = line_count;
        thread_data[i].sum = &total_sum;
        thread_data[i].mutex = &mutex;
        thread_data[i].current_index = &current_index;
        thread_data[i].cond = &cond;
        thread_data[i].thread_id = i;
        thread_data[i].total_threads = max_threads;
        
        pthread_create(&threads[i], NULL, process_numbers, &thread_data[i]);
    }
    
    for (int i = 0; i < max_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    __int128_t average = total_sum / line_count;
    
    print_results_table(max_threads, memory_limit, line_count, average);
    
    print_threads_info(getpid(), max_threads);
    
    print_str("\nНажмите Enter для завершения...");
    
    char enter;
    read(STDIN_FILENO, &enter, 1);
    
    for (int i = 0; i < line_count; i++) {
        free(lines[i]);
    }
    free(lines);
    
    return 0;
}