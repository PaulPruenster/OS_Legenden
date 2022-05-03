#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>

int counter = 1;

void *inkrement_counter_function(void *ptr)
{
	counter += 1;
	return NULL;
}

int main()
{
	pid_t wpid;
	int status = 0;
	pthread_t thread; 

	printf("%d\n", counter);
	if (fork() == 0)
	{
		counter += 1;
		exit(0);
	}
	while ((wpid = wait(&status)) > 0)
		;
	printf("%d\n", counter);

	pthread_create(&thread, NULL, inkrement_counter_function, NULL);
	pthread_join(thread, NULL);
	printf("%d\n", counter);
	return EXIT_SUCCESS;
}