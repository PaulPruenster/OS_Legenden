CFLAGS = -std=c11 -Wall -g -Wextra -Wpedantic -pthread #-fsanitize=address -Wconversion

.PHONY: all clean
all: task1a task1b
clean:
	$(RM) task1a task1b 

task1a: task1a.c
	$(CC) $(CFLAGS) -o $@ $^

task1b: task1b.c
	$(CC) $(CFLAGS) -o $@ $^

val:
	 valgrind --tool=memcheck ./task1b
