.PHONY: all clean
all: some_library my_program
clean:
	$(RM) some_library.so some_library.o my_program

my_program: my_program.c
	$(CC) -Wall -Werror -o $@ $^

some_library: some_library.c
	gcc -c -Wall -Werror -fpic some_library.c
	gcc -shared -o some_library.so some_library.o