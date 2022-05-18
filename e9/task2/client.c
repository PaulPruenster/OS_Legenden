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

typedef struct client_data {
    int sockfd;
    char * name; 
} client_data; 


void* write_to_server(void * arg)
{
    client_data * cdata = (client_data * ) arg;
    char buff[MAX];
    send(cdata->sockfd, cdata->name, strlen(cdata->name), 0);
    for (;;)
    {
        bzero(buff, sizeof(buff));
        printf("> ");
        
        int n = 0;
        while ((buff[n++] = getchar()) != '\n')
            ;
        if (strncmp("/quit", buff, 5) == 0)
        {
            perror("schaden");
            return NULL;
        }
        if (strncmp("/shutdown", buff, 9) == 0)
        {
            send(cdata->sockfd, buff, strlen(buff), 0);
            return NULL;
        }
        send(cdata->sockfd, buff, strlen(buff), 0);
        bzero(buff, sizeof(buff));
    }
}


void* read_from_server(void * arg)
{
    char *buff = malloc(MAX * sizeof(char));
    client_data * cdata = (client_data * ) arg;
    for (;;)
    {
        bzero(buff, sizeof(buff));
        if(!recv(cdata->sockfd, buff, MAX, 0)){
            free(buff);
            return NULL;
        }
        printf("%s", buff);
        fflush(stdout);
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
    
   // pthread_create(&data->clients[i].thread, NULL, job, (void *)c_data);
    client_data * cdata = malloc(sizeof(client_data));
    cdata->name = argv[2];
    cdata->sockfd = sockfd;
    pthread_create(&write_thread,NULL,write_to_server, (void * )cdata);
    pthread_create(&read_thread, NULL, read_from_server, (void * ) cdata);
    pthread_join(write_thread, NULL);
    pthread_join(read_thread, NULL);

    close(sockfd);
}
