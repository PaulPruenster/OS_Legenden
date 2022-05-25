CFLAGS = -std=c11 -Wall -Wextra -Wpedantic #-fsanitize=address -Wconversion

.PHONY: all clean
all: task3_server task3_client

clean:
	$(RM) task3_server task3_client

task3_server: task3_server.c
	$(CC) $(CFLAGS) -o $@ $^

task3_client: task3_client.c
	$(CC) $(CFLAGS) -o $@ $^ 
