CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -D_DEFAULT_SOURCE #-fsanitize=address -Wconversion

.PHONY: all
all: signal_handler

.PHONY: clean
clean:
	$(RM) signal_handler

signal_handler: signal_handler.c
	$(CC) $(CFLAGS) -o $@ $^	