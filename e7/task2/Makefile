CFLAGS = -Wall -Wextra -pedantic -std=c11 -pthread -O2

.PHONY: all clean
all: task2_My_Mutex_1
clean:
	$(RM) task2

task2_My_Mutex_1: task2.c
	$(CC) $(CFLAGS) -DUSE_MY_MUTEX=1 -o task2 $^ 

task2_My_Mutex_0: task2.c
	$(CC) $(CFLAGS) -DUSE_MY_MUTEX=0 -o task2 $^ 

