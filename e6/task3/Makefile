CFLAGS = -std=c11 -Wall -Wextra -O2 -g -Wpedantic -pthread #-fsanitize=address -Wconversion

.PHONY: all clean
all: task3
clean:
	$(RM) task3

task2: task3.c
	$(CC) $(CFLAGS) -o $@ $^
