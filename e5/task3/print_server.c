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

const int msgssize = 1000;

struct message
{
	char data[1000];
	bool quit;
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
	if (argc != 2)
	{
		printf("wrong amount of arguments!");
		return EXIT_FAILURE;
	}
	if (!create_message_queue(argv[1]))
	{
		printf("error creat mp\n");
		return EXIT_FAILURE;
	}
	const mqd_t mq = mq_open(argv[1], O_RDONLY, 0, NULL);
	if (mq == -1)
	{
		printf("error open mp");
		return EXIT_FAILURE;
	}
	bool finish = true;
	while (finish)
	{
		message msg = {0};
		unsigned int priority = 0;
		if (mq_receive(mq, (char *)&msg, sizeof(msg), &priority) == -1)
			return EXIT_FAILURE;

		if (msg.quit)
		{
			printf("Shutting down\n");
			finish = false;
			mq_close(mq);
			mq_unlink(argv[1]);
		}
		else
		{
			printf("Serving print job with priority %u:\n", priority);
			int i = 0;
			char a = msg.data[i];
			while (a != '\0' && i < msgssize)
			{
				putc(a, stdout);
				sleep(1);
				fflush(stdout);
				a = msg.data[++i];
			}
			putc('\n', stdout);
		}
	}
}