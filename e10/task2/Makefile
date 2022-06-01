CFLAGS = -Wall -Wextra -pedantic -pthread -g -O3

.PHONY: all clean
all: task2
clean:
	$(RM) task2 task2.o task2-optimized2 task2-optimized3 test

task2: task2.c membench.o allocator_tests.o
	$(CC) $(CFLAGS) -o $@ $^

test: test.c membench.o allocator_tests.o
	$(CC) $(CFLAGS) -o $@ $^

task2-optimized2: task2.c membench.o allocator_tests.o
	$(CC) $(CFLAGS) -o2 -o $@ $^ 

task2-optimized3: task2.c membench.o allocator_tests.o
	$(CC) $(CFLAGS) -o3 -o $@ $^ 

val:
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all -s ./task2