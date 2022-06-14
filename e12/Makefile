CFLAGS = -Wall -Wextra -pedantic -pthread -g

.PHONY: all clean
all: restaurant
clean:
	$(RM) restaurant restaurant.o restaurant-optimized2 restaurant-optimized3

restaurant: restaurant.c
	$(CC) $(CFLAGS) -o $@ $^

restaurant-optimized2: restaurant.c
	$(CC) $(CFLAGS) -o2 -o $@ $^ 

restaurant-optimized3: restaurant.c
	$(CC) $(CFLAGS) -o3 -o $@ $^ 

val:
	valgrind --tool=memcheck ./restaurant 1 5 3