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

#define MAX 100
#define SA struct sockaddr

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

int port; 
int sockfd, connfd;
static int STOP = 1;
// https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/

static void handler()
{
    close(sockfd);
    close(connfd);
    exit(0);
}

void func(int connfd)
{
    char buff[MAX];
    for (;;)
    {
        bzero(buff, MAX);
        // read the message from client and copy it in buffer
        if (!recv(connfd, buff, sizeof(buff), 0)){
            printf("Client disconnected\n");
            return;
        }

        // print buffer which contains the client contents
        printf("Echo: %s", buff);

        // 11 string with \n\0 and 9 without
        if (strlen(buff) == 11 && strncmp("/shutdown", buff, 9) == 0)
        {
            printf("Shutting down.\n");
            STOP = 0;
            return;
        }
        bzero(buff, MAX);
    }
}

int main(int argc, char ** argv)
{
    if (argc != 2) {
        perror("wrong amount of arguments");
    }
    port = atoi(argv[1]);
    struct sigaction sa = {.sa_handler = handler};
    sigaction(SIGINT, &sa, 0);

    struct sockaddr_in servaddr, cli;
    socklen_t peer_addr_size;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    CHECK(sockfd, == -1, "socket creation failed...\n");
    printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, port
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    // cheat that server can be rerun after close
    int opt_val = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);

    peer_addr_size = sizeof(struct sockaddr_in);
    CHECK(bind(sockfd, (SA *)&servaddr, sizeof(servaddr)), != 0, "Listen failed...\n");
    printf("Socket successfully binded..\n");
    while (STOP)
    {

        CHECK((listen(sockfd, 5)), != 0, "Listen failed...\n");

        printf("Listening on port %d.\n", port);

        // Accept the data packet from client and verification
        connfd = accept(sockfd, (SA *)&cli, &peer_addr_size);
        CHECK(connfd, < 0, "server accept failed...\n");
        printf("Client connected\n");

        func(connfd);

        close(connfd);
    }
    close(sockfd);
}
