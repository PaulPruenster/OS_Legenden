CFLAGS = -std=c11 -Wall -Wextra -Wpedantic #-fsanitize=address -Wconversion

.PHONY: all
all: ls_wc ls_grep_wc

.PHONY: clean
clean:
	$(RM) ls_wc ls_grep_wc

ls_wc: ls_wc.c
	$(CC) $(CFLAGS) -o $@ $^

ls_grep_wc: ls_grep_wc.c
	$(CC) $(CFLAGS) -o $@ $^
