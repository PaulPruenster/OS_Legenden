#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>

/*
inspired by: https://man7.org/linux/man-pages/man2/poll.2.html
*/

typedef struct Client {
	char *name;
	int id;
} client;

char *getClient(client *clients, int number, int pfds) {
	for (int i = 0; i < number; i++) {
		if (clients[i].id == pfds) {
			return clients[i].name;
		}
	}
	return "";
}
char *removeBlanks(char *input) {
	int count = 0;
	for (int i = 0; input[i]; i++) {
		if (input[i] != ' ') {
			input[count++] = input[i];
		}
	}
	input[count] = '\0';
	return input;
}
double evaluateString(char *src, bool *error){
	src = removeBlanks(src);
	int len = strlen(src);
	char *ptr1, *ptr2;
	double a, b;
	a = strtod(src, &ptr1);
	b = strtod((ptr1 + 1), &ptr2);
	if (ptr1 + 1 == ptr2 || src+len != ptr2){
        *error = true; 
        return 0; 
    }
	*error = false;
	switch (ptr1[0]) {
	case '+': return a + b;
	case '-': return a - b;
	case '/': return a / b;
	case '*': return a * b;
	}
	*error = true; // ERROR
	return 0;
}

int main(int argc, char *argv[]) {
	int nfds, num_open_fds;
	struct pollfd *pfds;

	if (argc < 2) {
		printf("Not enough arguments\n!");
		return EXIT_FAILURE;
	}
	char names[argc][100];
	/*
		make fifos
		starts with 1, 0 is the server's filename
	*/
	for (int i = 1; i < argc; i++) {
		sprintf(names[i], "/tmp/%s", argv[i]);
		// only user can read/write on the fifos
		mkfifo(names[i], 0600);
	}

	num_open_fds = nfds = argc - 1;
	pfds = calloc(nfds, sizeof(struct pollfd));
	if (pfds == NULL){
		return EXIT_FAILURE;
	}
		
	client clients[nfds];
	/* Open each file on command line, and add it 'pfds' array. */
	for (int j = 0; j < nfds; j++) {
		pfds[j].fd = open(names[j + 1], O_RDONLY);
		if (pfds[j].fd == -1) {
			printf("could not connect to %s\n", argv[j + 1]);
			num_open_fds--;
		}
		printf("%s connected.\n", argv[j + 1]);
		clients[j].name = argv[j + 1];
		clients[j].id = pfds[j].fd;
		pfds[j].events = POLLIN;
	}

	while (num_open_fds > 0) {
		int ready;

		ready = poll(pfds, nfds, -1);
		if (ready == -1) {
			printf("Error poll\n");
			return EXIT_FAILURE;
		}
		for (int j = 0; j < nfds; j++) {
			// really bad though, for some reason PIPE_BUF does work on my system, but somehow not on zid-gpl
			char buf[1000] = {0};
			if (pfds[j].revents != 0) {
				if (pfds[j].revents & POLLIN) {
					ssize_t s = read(pfds[j].fd, buf, sizeof(buf));
					if (s == -1) {
						printf("Error read\n");
						return EXIT_FAILURE;
					}
					bool error = false;
					double result = evaluateString(buf, &error);
					if (!error) {
						printf("%s:%s = %g\n", getClient(clients, argc, pfds[j].fd), buf, result);
					} else {
						printf("%s:%s is malformed.\n", getClient(clients, argc, pfds[j].fd), buf);
					}
					memset(buf, 0, sizeof buf);
				}
				else if (pfds[j].revents & POLLHUP) {
					printf("%s disconnected\n", getClient(clients, argc, pfds[j].fd));
					if (close(pfds[j].fd) == -1) {
						printf("Error close\n");
						return EXIT_FAILURE;
					}
					//delete fifos
					unlink(names[j + 1]);
					num_open_fds--;
				}
			}
		}
	}

	printf("All closed\n");
	return EXIT_SUCCESS;
}
