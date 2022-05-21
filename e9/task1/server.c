#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <strings.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <semaphore.h>

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
} myclient;

typedef struct server_data
{
    myclient *clients;
    atomic_int * clients_connected;
} server_data;

typedef struct client_thread_data
{
    server_data *server_data;
    int id;
} client_thread_data;

static void handler()
{
    close(sockfd);
    exit(0);
}

void *job(void *p)
{
    client_thread_data *data = (client_thread_data *)p;
    myclient *c = &(data->server_data->clients[data->id]); // katastrophe

    char *buff = malloc(MAX * sizeof(char));
    recv(c->connfd, buff, MAX, 0);
    strcpy(c->name, buff); // destination, source
    printf("%s connected\n", c->name);
    fflush(stdout);

    while (1)
    {
        if (!recv(c->connfd, buff, MAX, 0))
        {
            atomic_fetch_sub(data->server_data->clients_connected,1);
            printf("%s disconnected\n", c->name);
            close(c->connfd);
            free(p);
            free(buff);
            return NULL;
        }

        if (strncmp("/shutdown", buff, 9) == 0)
        {
            printf("fahler\n");
            fflush(stdout);
            pthread_cancel(listenthr); 
            printf("Shutting down.\n");
            close(c->connfd);
            atomic_fetch_sub(data->server_data->clients_connected,1);
            free(p);
            free(buff);
            return NULL;
        }
        
        printf("%s: %s", c->name, buff);
        fflush(stdout);    
    }
}

void *listener(void *arg)
{
    server_data *data = (server_data *)arg;

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
        data->clients[i].connfd = accept(sockfd, (SA *)&client, &peer_addr_size);
        if (data->clients[i].connfd < 0)
        {
            perror("server accept failed...\n");
            continue;
        }
        atomic_fetch_add(data->clients_connected, 1);

        // printf("Client connected\n");
        client_thread_data *c_data = calloc(1,sizeof(client_thread_data));
        if (c_data == NULL)
        {
            perror("Client data error");
            return NULL;
        }
        c_data->id = i;
        c_data->server_data = data;
        pthread_create(&data->clients[i].thread, NULL, job, (void *)c_data);
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
    myclient *clients = calloc(CLIENTS, sizeof(myclient));
    server_data *data = calloc(1, sizeof(server_data));

    atomic_int clients_connected = 0;
    data->clients_connected = &clients_connected;
    data->clients = clients;

    pthread_create(&listenthr, NULL, listener, (void *)data);
    pthread_join(listenthr, NULL);

    printf("Server is shutting down. Waiting for %d clients to be disconnected.\n", clients_connected);
    fflush(stdout);
    for (size_t i = 0; i < CLIENTS; i++)
        pthread_join(clients[i].thread, NULL);
    //geat nt?
    // TODO es gib a dprintf wos an buffer in an socket sendet
    free(clients); 
    free(data);
    close(sockfd); // close socked without SIGINT
    return EXIT_SUCCESS;
}
