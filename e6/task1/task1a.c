#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>

int counter = 1;

int main() {
	pid_t wpid;
	int status = 0;
	printf("%d\n", counter);
	if(fork() == 0) {
		counter += 1;
		exit(0);
	}
	while((wpid = wait(&status)) > 0);
	printf("%d\n", counter);
}