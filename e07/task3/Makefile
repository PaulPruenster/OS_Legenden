CFLAGS = -Wall -Wextra -pedantic -O3 -pthread -D_DEFAULT_SOURCE #-std=c11 we removed -std=c11, in order to compile on zid

.PHONY: all clean
all: task3
clean:
	$(RM) task3 task3.o 

task3: task3.c
	$(CC) $(CFLAGS) -o $@ $^
