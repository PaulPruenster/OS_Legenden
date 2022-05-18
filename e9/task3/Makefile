CFLAGS = -std=c11 -Wall -g -Wextra -Wpedantic -pthread #-fsanitize=address -Wconversion

.PHONY: all clean
all: task1
clean:
	$(RM) task1

task1: task1.c
	$(CC) $(CFLAGS) -o $@ $^

val:
	 valgrind --tool=memcheck --leak-check=full -s ./task1 8080
