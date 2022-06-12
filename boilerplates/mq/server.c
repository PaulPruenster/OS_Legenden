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
mqd_t mq;
static void handler() {
	//keine printf im Handler, sollte write sein. Oder noch besser mit globaler variable und in der schleife
	//(dafür braucht es noch flag beim recieve )
	write(STDOUT_FILENO,"\nShutting down\n",15);
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

int main()
{
	struct sigaction psa;
	psa.sa_handler = &handler;
	sigaction(SIGINT, &psa, NULL);

	NAME = "/mq_nameasdfasdfsdgertfdfg";
	
	if (!create_message_queue(NAME))
	{
		printf("error creat mp\n");
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
		//mq_receive blocks 
		if (mq_receive(mq, (char *)&msg, sizeof(msg), &priority) == -1)
			return EXIT_FAILURE;

		//print the priority
		printf("Serving print job with priority %u:\n", priority);
		//Printing the message received
		printf("%s\n", msg.data);
		
	}
	return EXIT_SUCCESS;
}