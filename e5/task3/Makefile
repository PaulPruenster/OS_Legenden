CFLAGS = -std=c11 -Wall -Wextra -Wpedantic #-fsanitize=address -Wconversion

.PHONY: all clean
all: task3_printer_server task3_printer

clean:
	$(RM) task3_printer_server task3_printer

print_server: print_server.c
	$(CC) $(CFLAGS) -o $@ $^ -D_POSIX_C_SOURCE -D_BSD_SOURCE -D_DEFAULT_SOURCE -lrt

task3_printer: task3_printer.c
	$(CC) $(CFLAGS) -o $@ $^ -lrt
