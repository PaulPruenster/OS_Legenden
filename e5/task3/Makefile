CFLAGS = -std=c11 -Wall -Wextra -Wpedantic #-fsanitize=address -Wconversion

.PHONY: all clean
all: print_server print

clean:
	$(RM) print_server print

print_server: print_server.c
	$(CC) $(CFLAGS) -o $@ $^ -D_POSIX_C_SOURCE -D_BSD_SOURCE -D_DEFAULT_SOURCE -lrt

print: print.c
	$(CC) $(CFLAGS) -o $@ $^ -lrt
