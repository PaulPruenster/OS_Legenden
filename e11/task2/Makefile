CFLAGS = -Wall -Wextra -g

lib = tripple.so cube.so abs.so

.PHONY: all clean
all: map_number  $(lib)
clean:
	$(RM) map_number *.so

map_number: map_number.c
	$(CC) $(CFLAGS) -o $@ $^ -ldl

%.so: %.c
	$(CC) -shared -fPIC $(CFLAGS) -o $@ $<