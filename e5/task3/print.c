// C program to implement one side of FIFO
// This side reads first, then reads
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

struct message {
	char data[1000];
};
typedef struct message message;

int main(int argc, char* argv[]) {

	//message msg1;
	// strcpy(msg1.data, argv[3]);
	// printf("%s\n",msg1.data);
	
	EXIT_SUCCESS;
	if(argc != 4) {
		printf("wrong amount of arguments!\n");
		return EXIT_FAILURE;
	}
	const mqd_t mq = mq_open(argv[1], O_WRONLY, 0, NULL);
	if(mq == -1) {
		printf("fehler open");
		return EXIT_FAILURE;
	}
	message msg = {0};
	strcpy(msg.data, argv[3]);
	//printf("%s\n", msg.data);
	//msg.data = argv[3];
	printf("%s", msg.data);
	mq_send(mq, (const char*)&msg, sizeof(msg), atoi(argv[2]));
	mq_close(mq);
}