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
char * NAME;
struct message {
	char data[MSGSIZE];
};
typedef struct message message;

int main() {
	NAME = "/mq_nameasdfasdfsdgertfdfg";
	const mqd_t mq = mq_open(NAME, O_WRONLY, 0, NULL);
	if(mq == -1) {
		printf("fehler open\n");
		return EXIT_FAILURE;
	}
	message msg = {.data="das ist eine Message"};
	mq_send(mq, (const char*)&msg, sizeof(msg), 1); // 1 is priority
	mq_close(mq);
}