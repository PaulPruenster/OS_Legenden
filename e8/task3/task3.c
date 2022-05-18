#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <strings.h>
#include <stdbool.h>

#include "pool.h"

#define MAX 1000
#define SA struct sockaddr
#define THREADS 5

int port;
struct sockaddr_in servaddr, client;
socklen_t peer_addr_size;
pthread_t listenthr;

int sockfd;
thread_pool *pool;
int STOP = 1;

static void handler()
{
    close(sockfd);
    pool_destroy(pool);
    exit(0);
}

void *job(void *p)
{
    usleep(100000);
    int connfd = *(int *)p;
    char *buff = malloc(MAX * sizeof(char));

    // read the message from client and copy it in buffer
    if (!recv(connfd, buff, MAX, 0))
    {
        printf("Client disconnected\n");
        return NULL;
    }

    char *ret;
    // 11 string with \n\0 and 9 without
    // skip first 4 character of message, they are always 'GET '
    if (strncmp("/shutdown", buff + 4, 9) == 0)
    {
        ret = "HTTP/1.1 200 OK\r\ncontent-type:text/html\r\ncontent-length:14\r\n\r\nShutting down!";
        send(connfd, ret, strlen(ret), 0);
        pool_destroy(pool);
        pthread_cancel(listenthr); // https://stackoverflow.com/questions/9763025/memory-leaked-with-pthread-cancel-itself
        printf("Shutting down.\n");


        // exit(0); // ? should we send a response to the client?
    }
    else{
        ret = "HTTP/1.1 200 OK\r\ncontent-type:text/html\r\ncontent-length:56\r\n\r\n<img src=\"\" alt=\"test\"/>";
        send(connfd, ret, strlen(ret), 0);
    }
    // lost hours due to invalid send-size: 1.5 (@Benno & @Paul)
    close(connfd);
    free(p);
    free(buff);
    return NULL;
}

void *func()
{
    if (listen(sockfd, 10) != 0) // 10 is the maximum size of queue of listen()
    { 
        perror("Listen failed...\n");
        return NULL;
    }
    printf("Listening on port %d.\n", port);
    while (1)
    {
        // Accept the data packet from client and verification
        int *connfd = malloc(sizeof(int));
        *connfd = accept(sockfd, (SA *)&client, &peer_addr_size);
        if (*connfd < 0)
        {
            perror("server accept failed...\n");
            return NULL;
        }
        printf("Client connected\n");
        pool_submit(pool, job, (void *)connfd);
    }
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        perror("wrong amount of arguments!");
        return EXIT_FAILURE;
    }
    port = atoi(argv[1]);
    struct sigaction sa = {.sa_handler = handler};
    sigaction(SIGINT, &sa, 0);

    // create socket and verificate it
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("create socket failed...\n");
        return EXIT_FAILURE;
    }
    printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr)); // fill buffer with 0

    // assign IP, port
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    // cheat that server can be rerun after close
    int opt_val = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);

    peer_addr_size = sizeof(struct sockaddr_in);
    if (bind(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
    {
        perror("Binding failed...\n");
        return EXIT_FAILURE;
    }
    printf("Socket successfully binded..\n");

    pool = malloc(sizeof(thread_pool));
    pool_create(pool, THREADS);
    pthread_create(&listenthr, NULL, func, NULL);
    pthread_join(listenthr, NULL);
}
