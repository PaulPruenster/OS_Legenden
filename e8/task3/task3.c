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

#define MAX 1000 * 100
#define PORT 42069
#define SA struct sockaddr

#define THREADS 5

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

static int STOP = 1;
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
    /*
    For each client that connects, a connection handler function is executed:

      [x] Each connection handler function should be run in your thread pool from task 1.
      [ ] To simulate a real workload, the handler begins by sleeping for 100 milliseconds.
      [ ] The handler then "parses" the incoming HTTP request and responds accordingly (see below).

    */
    useconds_t s = 100000;
    usleep(s);

    char* message = (char *)arg;
    printf("%s\n", message);
    fflush(stdout);

    char* ret = "HTTP/1.1 200 OK\r\ncontent-type:text/html\r\ncontent-length:4\r\n\r\ntest";

    // lost hours to invalid send size: 1.5
    // times 10 seems to be the *magical* number, it started to worked, so we don't change anything from now on
    int error = send(connfd, ret, sizeof(ret) * 10, 0);
    printf

    //usleep(100);

    /*
    HTTP/1.1 200 OK\r\n
    Content-Type: text/html\r\n
    Content-Length: <number of bytes in message body, including newlines>\r\n
    \r\n
    Echo: <message>
    */
    return NULL;
}

void func(int connfd)
{
    char *buff = malloc(MAX * sizeof(char));
    for (;;)
    {
        bzero(buff, MAX);
        // read the message from client and copy it in buffer
        if (!recv(connfd, buff, sizeof(buff), 0))
        {
            printf("Client disconnected\n");
            return;
        }

        pool_submit(pool, job, (void *)buff);

        // print buffer which contains the client contents
        //printf("Echo: %s", buff);

        // 11 string with \n\0 and 9 without
        if (strlen(buff) == 11 && strncmp("/shutdown", buff, 9) == 0)
        {
            printf("Shutting down.\n");
            STOP = 0;
            return;
        }
        bzero(buff, MAX);
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
