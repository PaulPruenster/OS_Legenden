CFLAGS = -Wall -Wextra -pedantic -g -pthread -std=c11 -D_POSIX_C_SOURCE -D_BSD_SOURCE -D_DEFAULT_SOURCE#-std=c11 we removed -std=c11, in order to compile on zid

.PHONY: all clean
all: server client
clean:
	$(RM) server client 

server: server.c
	$(CC) $(CFLAGS) -o $@ $^ -lrt

client: client.c
	$(CC) $(CFLAGS) -o $@ $^ -lrt

val_server:
	 valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all -s ./server 8080

val_client:
	 valgrind --tool=memcheck --leak-check=full  --show-leak-kinds=all -s ./client 8080 "amongus"