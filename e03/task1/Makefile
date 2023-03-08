CFLAGS = -std=c11 -Wall -Wextra -Wpedantic #-fsanitize=address -Wconversion

.PHONY: all clean
all: task1

clean:
	$(RM) task1

task1: task1.c
	$(CC) $(CFLAGS) -o $@ $^
 