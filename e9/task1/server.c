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

#define MAX 1000
#define SA struct sockaddr
#define THREADS 5
#define CLIENTS 100

int port;
struct sockaddr_in servaddr, client;
socklen_t peer_addr_size;
pthread_t listenthr;

int sockfd;

typedef struct myclient_struct
{
    pthread_t thread;
    int connfd;
    char name[100];
    bool finished;
} myclient;

static void handler()
{
    close(sockfd);
    exit(0);
}

void *job(void *p)
{
    myclient *c = ((myclient *)p);
    char *buff = malloc(MAX * sizeof(char));
    recv(c->connfd, buff, MAX, 0);
    strcpy(c->name, buff); // destination, source
    printf("%s connected\n",c->name);
    fflush(stdout);

    while (1)
    {
        if (!recv(c->connfd, buff, MAX, 0))
        {
            free(p);
            free(buff);
            printf("Client disconnected\n");
            c->finished = true;
            return NULL;
        }

        // char *ret;
        if (strlen(buff) == 11 && strncmp("/shutdown", buff, 9) == 0)
        {
            pthread_cancel(listenthr); // https://stackoverflow.com/questions/9763025/memory-leaked-with-pthread-cancel-itself
            printf("Shutting down.\n");
            close(c->connfd);
            c->finished = true;
            free(p);
            free(buff);
            return NULL;
        }
        printf("%s: %s\n", c->name, buff);
        fflush(stdout);
    }
}

void *listener(void *arg)
{
    myclient *clients = (myclient *)arg;

    if (listen(sockfd, 10) != 0) // 10 is the maximum size of queue of listen()
    {
        perror("Listen failed...\n");
        return NULL;
    }
    printf("Listening on port %d.\n", port);
    int i = 0;
    while (1)
    {
        // Accept the data packet from client and verification
        // int *connfd = malloc(sizeof(int));
        clients[i].connfd = accept(sockfd, (SA *)&client, &peer_addr_size);
        if (clients[i].connfd < 0)
        {
            perror("server accept failed...\n");
            continue;
        }
        //printf("Client connected\n");

        pthread_create(&(clients[i].thread), NULL, job, (void *)&clients[i]);
        ++i;
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
    myclient *clients = malloc(sizeof(myclient) * CLIENTS);
    pthread_create(&listenthr, NULL, listener, (void *)clients);
    pthread_join(listenthr, NULL);
    int counter = 0;
    for (size_t i = 0; i < CLIENTS; i++)
    {
        if(clients[i].finished == false){
            counter++;
        }
    }
    printf("Server is shutting down. Waiting for %d clients to disconnected.", counter);
     for (size_t i = 0; i < CLIENTS; i++)
    {
        pthread_join(clients[i].thread, NULL);
    }
    

    return EXIT_SUCCESS;
}
