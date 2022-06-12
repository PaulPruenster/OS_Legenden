CFLAGS = -std=c11 -Wall -Wextra -Wpedantic #-fsanitize=address -Wconversion

.PHONY: all clean
all: server client

clean:
	$(RM) server client

server: server.c
	$(CC) $(CFLAGS) -o $@ $^ -D_POSIX_C_SOURCE -D_BSD_SOURCE -D_DEFAULT_SOURCE -lrt

client: client.c
	$(CC) $(CFLAGS) -o $@ $^ -lrt
