CFLAGS = -Wall -Wextra -pedantic -pthread -g -O3

.PHONY: all clean
all: task2
clean:
	$(RM) task2

task2: task2.c
	$(CC) $(CFLAGS) -o $@ $^