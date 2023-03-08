CFLAGS = -std=c11 -Wall -Wextra -Wpedantic #-fsanitize=address -Wconversion

.PHONY: all clean
all: even

clean:
	$(RM) even

even: even.c
	$(CC) $(CFLAGS) -o $@ $^