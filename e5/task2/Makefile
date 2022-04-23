CFLAGS = -std=c11 -Wall -Wextra -Wpedantic #-fsanitize=address -Wconversion

.PHONY: all
all: task2

.PHONY: clean
clean:
	$(RM) task2.o task2

task2: task2.c
	$(CC) $(CFLAGS) -o $@ $^ -lrt