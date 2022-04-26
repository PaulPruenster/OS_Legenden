// C program to implement one side of FIFO
// This side reads first, then reads
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <mqueue.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#define MSGSIZE 1000

volatile bool STOP = false;
char * NAME;
char *color[] = {"\033[1;31m", "\033[31;1m", "\033[0;33m", "\033[0;32m","\033[0;34m","\033[0;36m"};
mqd_t mq;
static void handler() {
	printf("\033[0;34m");
	printf("\nShutting down\n");
	mq_close(mq);
	mq_unlink(NAME);
	exit(0);
}

struct message
{
	char data[MSGSIZE];
};
typedef struct message message;

bool create_message_queue(const char *name)
{
	const int oflag = O_CREAT | O_EXCL;
	const mode_t permissions = S_IRUSR | S_IWUSR; // 600
	const struct mq_attr attr = {.mq_maxmsg = 10, .mq_msgsize = sizeof(message)};
	const mqd_t mq = mq_open(name, oflag, permissions, &attr);
	if (mq == -1)
		return false;
	mq_close(mq);
	return true;
}

int main(int argc, char *argv[])
{
	printf("\033[0;34m");
	struct sigaction psa;
	psa.sa_handler = &handler;
	sigaction(SIGINT, &psa, NULL);

	if (argc != 2)
	{
		printf("wrong amount of arguments!\n");
		return EXIT_FAILURE;
	}
	char name[1000];
	if (argv[1][0] == '/')
		sprintf(name, "/csaz9531%s", argv[1] + 1);
	else
		sprintf(name, "/csaz9531%s", argv[1]);

	NAME = name;

	if (!create_message_queue(NAME))
	{
		printf("error creat mp ( '/' gets added by programm)\n");
		return EXIT_FAILURE;
	}
	fprintf(stdout, "%s created\n", NAME);
	fflush(stdout);

	mq = mq_open(NAME, O_RDONLY, 0, NULL);
	if (mq == -1)
	{
		printf("error open mp\n");
		return EXIT_FAILURE;
	}
	while (true)
	{
		message msg = {0};
		unsigned int priority = 0;

		if (mq_receive(mq, (char *)&msg, sizeof(msg), &priority) == -1)
			return EXIT_FAILURE;

		printf("Serving print job with priority %u:\n", priority);
		int i = 0;
		char a = msg.data[i];
		while (a != '\0' && i < MSGSIZE)
		{
			printf("%s", color[i % 6]);
			putc(a, stdout);
			usleep(200000);
			fflush(stdout);
			a = msg.data[++i];
		}
		printf("\033[0;34m");
		//new Line if  last Character is not '\n'
		if(msg.data[i - 1] != '\n' && msg.data[i] != '\n')

			putc('\n', stdout);
	}
	return EXIT_SUCCESS;
}