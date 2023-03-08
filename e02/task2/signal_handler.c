#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void handler(int signo) {
	switch(signo) {
		//printf is not signal save, it creats a deadlock, so using the asyncrone write is the best option.
		case SIGINT: write(1, "SIGINT\n", 8); break;
		case SIGUSR1: write(1, "SIGUSR1\n", 9); break;
		case SIGTERM: write(1, "SIGTERM\n", 9); break;
		case SIGKILL: write(1, "SIGTERM\n", 9); break;
	}
	return;
}

int main() {
	struct sigaction psa;
	psa.sa_handler = &handler;
	sigaction(SIGINT, &psa, NULL);
	sigaction(SIGUSR1, &psa, NULL);
	sigaction(SIGTERM, &psa, NULL);
	sigaction(SIGKILL, &psa, NULL);
	for(;;) {
		usleep(1000);
	}
	return EXIT_SUCCESS;
}