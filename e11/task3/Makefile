CFLAGS = -Wall -Wextra -g

.PHONY: all clean
all: malloc_spy.so
clean:
	$(RM) *.so

malloc_spy.so: malloc_spy.c
	$(CC) -shared -fPIC $(CFLAGS) -o $@ $< -ldl