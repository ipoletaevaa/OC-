CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Werror -pedantic
LDFLAGS = -lrt -lpthread

TARGETS = parent child1 child2
SOURCES = parent.c child1.c child2.c
HEADERS = common.h

all: $(TARGETS)

parent: parent.c common.h
	$(CC) $(CFLAGS) -o parent parent.c $(LDFLAGS)

child1: child1.c common.h
	$(CC) $(CFLAGS) -o child1 child1.c $(LDFLAGS)

child2: child2.c common.h
	$(CC) $(CFLAGS) -o child2 child2.c $(LDFLAGS)

clean:
	rm -f $(TARGETS)

rebuild: clean all

run: all
	@echo "=== Запуск Lab 3 ==="
	@echo "1. Соберите: make"
	@echo "2. Запустите: ./parent"
	@echo "3. Введите имена файлов для child1 и child2"
	@echo "4. Вводите строки для обработки"

.PHONY: all clean rebuild run