CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Werror -pedantic -fsanitize=address
LDFLAGS = -fsanitize=address -pthread

TARGET = lab2
SOURCES = main.c calculator.c
OBJECTS = $(SOURCES:.c=.o)
HEADERS = calculator.h

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJECTS)

rebuild: clean $(TARGET)

run: $(TARGET)
	./$(TARGET)

test: $(TARGET)
	./$(TARGET) -f numbers.txt -t 4 -m 100

debug: CFLAGS += -g -O0
debug: rebuild

profile: CFLAGS += -pg
profile: LDFLAGS += -pg
profile: rebuild

install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/

help:
	@echo "Доступные цели:"
	@echo "  all       - сборка программы"
	@echo "  clean     - удаление скомпилированных файлов"
	@echo "  rebuild   - полная пересборка"
	@echo "  run       - запуск с параметрами по умолчанию"
	@echo "  test      - запуск с тестовыми параметрами"
	@echo "  debug     - отладочная сборка"
	@echo "  profile   - сборка для профилирования"
	@echo "  install   - установка в систему"
	@echo "  help      - это сообщение"

all: $(TARGET)

.PHONY: all clean rebuild run test debug profile install help