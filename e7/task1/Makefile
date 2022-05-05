CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -pthread #-fsanitize=address -Wconversion

.PHONY: all clean
all: task1
clean:
	$(RM) task1 task1.o task1-optimized2 task1-optimized3

task1: task1.c
	$(CC) $(CFLAGS) -o $@ $^

task1-optimized2: task1.c
	$(CC) $(CFLAGS) -o2 -o $@ $^ 

task1-optimized3: task1.c
	$(CC) $(CFLAGS) -o3 -o $@ $^ 
