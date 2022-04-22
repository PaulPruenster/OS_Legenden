CFLAGS = -std=c11 -Wall -Wextra -Wpedantic #-fsanitize=address -Wconversion

.PHONY: all
all: task1

.PHONY: clean
clean:
	$(RM) task1.o task1

task1: task1.c
	$(CC) $(CFLAGS) -o $@ $^