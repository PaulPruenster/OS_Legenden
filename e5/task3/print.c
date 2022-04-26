#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <mqueue.h>
#include <pthread.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define MSGSIZE 1000

struct message {
	char data[MSGSIZE];
};
typedef struct message message;

int main(int argc, char* argv[]) {
	
	if(argc != 3) {
		printf("wrong amount of arguments!\n");
		return EXIT_FAILURE;
	}
	const mqd_t mq = mq_open(argv[1], O_WRONLY, 0, NULL);
	if(mq == -1) {
		printf("fehler open\n");
		return EXIT_FAILURE;
	}
	message msg = {0};
	fgets( msg.data, MSGSIZE, stdin);
	mq_send(mq, (const char*)&msg, sizeof(msg), atoi(argv[2]));
	mq_close(mq);
}