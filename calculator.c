#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "calculator.h"

__int128_t hex_to_int128(const char *hex) {
    __int128_t result = 0;
    
    if (hex[0] == '0' && (hex[1] == 'x' || hex[1] == 'X')) {
        hex += 2;
    }
    
    while (*hex) {
        char c = *hex++;
        result <<= 4;
        
        if (c >= '0' && c <= '9') {
            result |= (c - '0');
        } else if (c >= 'A' && c <= 'F') {
            result |= (c - 'A' + 10);
        } else if (c >= 'a' && c <= 'f') {
            result |= (c - 'a' + 10);
        }
    }
    
    return result;
}

void* process_numbers(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    
    while (1) {
        pthread_mutex_lock(data->mutex);
        
        int index = *(data->current_index);
        if (index >= data->count) {
            pthread_mutex_unlock(data->mutex);
            break;
        }
        
        *(data->current_index) = index + 1;
        pthread_mutex_unlock(data->mutex);
        
        __int128_t number = hex_to_int128(data->lines[index]);
        
        pthread_mutex_lock(data->mutex);
        *(data->sum) += number;
        pthread_mutex_unlock(data->mutex);
    }
    
    return NULL;
}

char** read_file_with_memory_limit(const char *filename, int memory_limit_mb, int *line_count) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        return NULL;
    }
    
    struct stat st;
    if (fstat(fd, &st) == -1) {
        close(fd);
        return NULL;
    }
    
    size_t file_size = st.st_size;
    size_t memory_limit = (size_t)memory_limit_mb * 1024 * 1024;
    
    if (file_size > memory_limit) {
        close(fd);
        return NULL;
    }
    
    char *file_content = malloc(file_size + 1);
    if (!file_content) {
        close(fd);
        return NULL;
    }
    
    ssize_t bytes_read = read(fd, file_content, file_size);
    if (bytes_read != (ssize_t)file_size) {
        free(file_content);
        close(fd);
        return NULL;
    }
    file_content[file_size] = '\0';
    close(fd);
    
    *line_count = 0;
    char *ptr = file_content;
    while (*ptr) {
        if (*ptr == '\n') (*line_count)++;
        ptr++;
    }
    
    if (file_size > 0 && file_content[file_size - 1] != '\n') {
        (*line_count)++;
    }
    
    if (*line_count == 0) {
        free(file_content);
        return NULL;
    }
    
    char **lines = malloc(*line_count * sizeof(char*));
    if (!lines) {
        free(file_content);
        return NULL;
    }
    
    int line_index = 0;
    char *line_start = file_content;
    char *current = file_content;
    
    while (*current && line_index < *line_count) {
        if (*current == '\n') {
            size_t line_length = current - line_start;
            lines[line_index] = malloc(line_length + 1);
            if (!lines[line_index]) {
                for (int i = 0; i < line_index; i++) free(lines[i]);
                free(lines);
                free(file_content);
                return NULL;
            }
            memcpy(lines[line_index], line_start, line_length);
            lines[line_index][line_length] = '\0';
            line_index++;
            line_start = current + 1;
        }
        current++;
    }
    
    if (line_index < *line_count && line_start < current) {
        size_t line_length = current - line_start;
        lines[line_index] = malloc(line_length + 1);
        if (!lines[line_index]) {
            for (int i = 0; i < line_index; i++) free(lines[i]);
            free(lines);
            free(file_content);
            return NULL;
        }
        memcpy(lines[line_index], line_start, line_length);
        lines[line_index][line_length] = '\0';
    }
    
    free(file_content);
    return lines;
}