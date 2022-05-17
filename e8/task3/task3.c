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
#define PORT 42069
#define SA struct sockaddr

#define THREADS 5
// TODO: we definitely need to refactor this:
#define CHECK(x, compare, errormessage)                                                                  \
    do                                                                                                   \
    {                                                                                                    \
        int retval = (x);                                                                                \
        if (retval compare)                                                                              \
        {                                                                                                \
            fprintf(stderr, "%s", errormessage);                                                         \
            fprintf(stderr, "Runtime error: %s returned %d at %s:%d\n", #x, retval, __FILE__, __LINE__); \
            close(connfd);                                                                               \
            close(sockfd);                                                                               \
            return EXIT_FAILURE;                                                                         \
        }                                                                                                \
    } while (0)

int sockfd, connfd;
thread_pool *pool;
int STOP = 1;
// https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/

static void handler()
{
    close(sockfd);
    close(connfd);
    pool_destroy(pool);
    exit(0);
}

void *job(void *arg)
{
    usleep(100000);

    char *message = (char *)arg;

    // 11 string with \n\0 and 9 without
    // skip first 4 character of message, they are always 'GET '
    if (strncmp("/shutdown", message + 4, 9) == 0)
    {
        printf("Shutting down.\n");
        exit(0);
    }

    char *ret = "HTTP/1.1 200 OK\r\ncontent-type:text/html\r\ncontent-length:56\r\n\r\n<img src=\"https://i.imgflip.com/68ok5u.jpg\" alt=\"test\"/>";

    // lost hours due to invalid send-size: 1.5 (@Benno & @Paul)
    int error = send(connfd, ret, strlen(ret), 0);
    printf("Send: %i\n", error);
    fflush(stdout);
    free(arg);
    return NULL;
}

void func(int connfd)
{
    char *buff = malloc(MAX * sizeof(char));
    while (STOP)
    {
        // read the message from client and copy it in buffer
        if (!recv(connfd, buff, MAX, 0))
        {
            printf("Client disconnected\n");
            return;
        }

        // add first line of request to arg
        char *arg = strtok(buff, "\n");

        pool_submit(pool, job, (void *)arg);
    }
    free(buff);
}

int main()
{
    struct sigaction sa = {.sa_handler = handler};
    sigaction(SIGINT, &sa, 0);

    struct sockaddr_in servaddr, client;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    CHECK(sockfd, == -1, "socket creation failed...\n");
    printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr)); // fill buffer with 0

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    // cheat that server can be rerun after close
    int opt_val = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);

    socklen_t peer_addr_size = sizeof(struct sockaddr_in);
    CHECK(bind(sockfd, (SA *)&servaddr, sizeof(servaddr)), != 0, "Listen failed...\n");
    printf("Socket successfully binded..\n");

    pool = malloc(sizeof(thread_pool));
    pool_create(pool, THREADS);

    while (STOP)
    {

        CHECK((listen(sockfd, 5)), != 0, "Listen failed...\n");

        printf("Listening on port %d.\n", PORT);

        // Accept the data packet from client and verification
        connfd = accept(sockfd, (SA *)&client, &peer_addr_size);
        CHECK(connfd, < 0, "server accept failed...\n");
        printf("Client connected\n");

        func(connfd);

        close(connfd);
    }
    close(sockfd);
    pool_destroy(pool);
}
