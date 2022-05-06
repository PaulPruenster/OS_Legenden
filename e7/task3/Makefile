CFLAGS = -Wall -Wextra -g -pedantic -std=c11 -O3 -pthread -D_DEFAULT_SOURCE

.PHONY: all clean
all: task3
clean:
	$(RM) task3 task3.o task3-optimized2 task3-optimized3

task3: task3.c
	$(CC) $(CFLAGS) -o $@ $^

task3-optimized2: task3.c
	$(CC) $(CFLAGS) -o2 -o $@ $^ 

task3-optimized3: task3.c
	$(CC) $(CFLAGS) -o3 -o $@ $^ 
