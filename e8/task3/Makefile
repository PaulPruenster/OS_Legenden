CFLAGS = -Wall -Wextra -pedantic -g -std=c11 #-D_POSIX_C_SOURCE -D_BSD_SOURCE -D_DEFAULT_SOURCE#-std=c11 we removed -std=c11, in order to compile on zid

.PHONY: all clean
all: task3
clean:
	$(RM) task3 task3.o 

task3: task3.c
	$(CC) $(CFLAGS) -o $@ $^ 

val:
	 valgrind --tool=memcheck --leak-check=full -s ./task3
