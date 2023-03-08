CFLAGS = -std=c11 -Wall -Wextra -Wpedantic #-fsanitize=address -Wconversion

.PHONY: all clean
all: read_my_env_var

clean:
	$(RM) read_my_env_var

read_my_env_var: read_my_env_var.c
	$(CC) $(CFLAGS) -o $@ $^
 