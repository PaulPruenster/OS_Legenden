CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -pthread #-fsanitize=address -Wconversion

.PHONY: all clean
all: task2
clean:
	$(RM) task2

task2: task2.c
	$(CC) $(CFLAGS) -o $@ $^
