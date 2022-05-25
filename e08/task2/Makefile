CFLAGS = -Wall -Wextra -pedantic  -std=c11 -D_POSIX_C_SOURCE -D_BSD_SOURCE -D_DEFAULT_SOURCE#-std=c11 we removed -std=c11, in order to compile on zid

.PHONY: all clean
all: task2
clean:
	$(RM) task2 task2.o 

task2: task2.c
	$(CC) $(CFLAGS) -o $@ $^  -lrt

val:
	 valgrind --tool=memcheck --leak-check=full -s ./task2
