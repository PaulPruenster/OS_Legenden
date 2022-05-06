CFLAGS = -Wall -Wextra -pedantic -std=c11 -pthread -D_DEFAULT_SOURCE

.PHONY: all clean
all: task3
clean:
	$(RM) task3 task3.o

task3: task3.c
	$(CC) $(CFLAGS) -o $@ $^
