CFLAGS = -Wall -Wextra -pedantic -pthread -g -O3

.PHONY: all clean
all: task1
clean:
	$(RM) task1 task1.o task1-optimized2 task1-optimized3

task1: task1.c
	$(CC) $(CFLAGS) -o $@ $^

test: test.c
	$(CC) $(CFLAGS) -o $@ $^

task1-optimized2: task1.c
	$(CC) $(CFLAGS) -o2 -o $@ $^ 

task1-optimized3: task1.c
	$(CC) $(CFLAGS) -o3 -o $@ $^ 

val:
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all -s ./task1