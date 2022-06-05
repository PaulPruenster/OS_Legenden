CFLAGS = -Wall -Wextra -g

lib = tripple.so cube.so abs.so

.PHONY: all clean
all: task2  $(lib)
clean:
	$(RM) task2 *.so

task2: task2.c
	$(CC) $(CFLAGS) -o $@ $^ -ldl

%.so: %.c
	$(CC) -shared -fPIC $(CFLAGS) -o $@ $<