#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>

#define MAX 1000

pthread_t write_thread;
pthread_t read_thread;

typedef struct client_data
{
    int sockfd;
    char *name;
} client_data;

void *write_to_server(void *arg)
{
    client_data *cdata = (client_data *)arg;
    char buff[MAX] = {0};
    send(cdata->sockfd, cdata->name, strlen(cdata->name), 0);
    for (;;)
    {
        bzero(buff, sizeof(buff));
        int n = 0;
        while ((buff[n++] = getchar()) != '\n')
            ;

        //if /quit is the message, the clients disconnects
        if (strncmp("/quit", buff, 5) == 0)
        {

            fflush(stdout);
            pthread_cancel(read_thread);
            return NULL;
        }
        send(cdata->sockfd, buff, strlen(buff), 0);
        bzero(buff, sizeof(buff));
    }
}

void *read_from_server(void *arg)
{
    char buff[MAX] = {0};
    client_data *cdata = (client_data *)arg;
    while (1)
    {
        bzero(buff, strlen(buff));
        if (!recv(cdata->sockfd, buff, MAX, 0)) {
            pthread_cancel(write_thread);
            break;
        }
        printf("%s", buff);
        fflush(stdout);
    }
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        perror("wrong amount of arguments!");
        return EXIT_FAILURE;
    }
    int port = atoi(argv[1]);
    if (port < 0 || port > 65536)
    {
        fprintf(stderr, "Port: %d does not exist!", port);
        return EXIT_FAILURE;
    }

    int sockfd;
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket creation failed...\n");
        return EXIT_FAILURE;
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
        perror("connection with the server failed...\n");
        return EXIT_FAILURE;
    }
    else
        printf("connected to the server..\n");

    client_data *cdata = malloc(sizeof(client_data));
    cdata->name = argv[2];
    cdata->sockfd = sockfd;
    pthread_create(&write_thread, NULL, write_to_server, (void *)cdata);
    pthread_create(&read_thread, NULL, read_from_server, (void *)cdata);
    pthread_join(write_thread, NULL);
    pthread_join(read_thread, NULL);
    free(cdata);
    close(sockfd);
}
