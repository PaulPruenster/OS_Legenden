CFLAGS = -std=c11 -Wall -g -Wextra -Wpedantic #-fsanitize=address -Wconversion

.PHONY: all
all: task2

.PHONY: clean
clean:
	$(RM) task2.o task2

unlink: unlink.c
	gcc -Wall unlink.c -o unlink -lrt

task2: task2.c
	$(CC) $(CFLAGS) -o $@ $^ -lrt -pthread