CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -D_DEFAULT_SOURCE#-fsanitize=address -Wconversion

.PHONY: all clean
all: print_server print

clean:
	$(RM) print_server print

print_server: print_server.c
	$(CC) $(CFLAGS) -o $@ $^ -lrt

print: print.c
	$(CC) $(CFLAGS) -o $@ $^ -lrt
