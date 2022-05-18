#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#define MAX 80

void func(int sockfd, char *name)
{
    char buff[MAX];
    send(sockfd, name, strlen(name), 0);
    for (;;)
    {
        bzero(buff, sizeof(buff));
        // scanf("%s", buff);
        int n = 0;
        printf("> ");
        while ((buff[n++] = getchar()) != '\n')
            ;
        if (strncmp("/quit", buff, 5) == 0)
        {
            perror("schaden");
            return;
        }
        send(sockfd, buff, sizeof(buff), 0);
        bzero(buff, sizeof(buff));
    }
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        perror("wrong amount of arguments!");
        return EXIT_FAILURE;
    }
    int port = atoi(argv[1]);
    int sockfd;
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    servaddr.sin_port = htons(port);
    // connect the client socket to server socket
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");

    func(sockfd, argv[2]);

    close(sockfd);
}
