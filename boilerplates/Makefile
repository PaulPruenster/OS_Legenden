CFLAGS = -Wall -Wextra -pedantic -std=c11 -pthread -g

.PHONY: all clean
all: mutex_cond_threads_queue
clean:
	$(RM) mutex_cond_threads_queue mutex_cond_threads_queue.o mutex_cond_threads_queue-optimized2 mutex_cond_threads_queue-optimized3

mutex_cond_threads_queue: mutex_cond_threads_queue.c
	$(CC) $(CFLAGS) -o $@ $^

mutex_cond_threads_queue-optimized2: mutex_cond_threads_queue.c
	$(CC) $(CFLAGS) -o2 -o $@ $^ 

mutex_cond_threads_queue-optimized3: mutex_cond_threads_queue.c
	$(CC) $(CFLAGS) -o3 -o $@ $^ 

val:
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all -s ./mutex_cond_threads_queue