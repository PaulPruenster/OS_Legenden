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
    atomic_bool isAlive;
    char name[100];
} myclient;

typedef struct server_data
{
    myclient *clients;
    atomic_int *clients_connected;
    atomic_int *clients_counter;

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

void *job(void *arg)
{
    client_thread_data *data = (client_thread_data *)arg;
    myclient *c = &(data->server_data->clients[data->id]);

    char *buff = malloc(MAX * sizeof(char));
    recv(c->connfd, buff, MAX, 0);
    strcpy(c->name, buff); // destination, source
    printf("%s connected\n", c->name);
    fflush(stdout);

    while (1)
    {
        bzero(buff, strlen(buff));
        if (!recv(c->connfd, buff, MAX, 0))
        {
            atomic_fetch_sub(data->server_data->clients_connected, 1);
            printf("%s disconnected\n", c->name);
            close(c->connfd);
            c->isAlive = false;
            free(arg);
            free(buff);
            return NULL;
        }

        if (strncmp("/shutdown", buff, 9) == 0)
        {
            c->isAlive = false;
            fflush(stdout);
            pthread_cancel(listenthr);
            printf("Shutting down.\n");
            close(c->connfd);
            atomic_fetch_sub(data->server_data->clients_connected, 1);
            free(arg);
            free(buff);
            return NULL;
        }

        char *message = malloc(sizeof(char) * MAX);
        sprintf(message, "%s: %s", c->name, buff);

        int counter_active_clients = *data->server_data->clients_connected;
        int i = 0;
        while (counter_active_clients > 0 && i < CLIENTS)
        {
            if (data->server_data->clients[i].isAlive)
            {
                counter_active_clients--;
                if (i != data->id)
                    send(data->server_data->clients[i].connfd, message, strlen(message), 0);
            }
            i++;
        }

        free(message);

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

        data->clients[i].connfd = accept(sockfd, (SA *)&client, &peer_addr_size);
        data->clients[i].isAlive = true;
        if (data->clients[i].connfd < 0)
        {
            data->clients[i].isAlive = false;
            perror("server accept failed...\n");
            continue;
        }
        atomic_fetch_add(data->clients_connected, 1);
        atomic_fetch_add(data->clients_counter, 1);

        // printf("Client connected\n");
        client_thread_data *c_data = calloc(1, sizeof(client_thread_data));
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

    // create socket and verifies it
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

    // prevent the os to lock the port and to be able to restart the server instantly 

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
    atomic_int clients_counter = 0;

    data->clients_connected = &clients_connected;
    data->clients_counter = &clients_counter;
    data->clients = clients;

    pthread_create(&listenthr, NULL, listener, (void *)data);
    pthread_join(listenthr, NULL);

    printf("Server is shutting down. Waiting for %d clients to disconnected.\n", clients_connected);
    fflush(stdout);
    size_t max_clients = *data->clients_counter;
    for (size_t i = 0; i < max_clients; i++)
        pthread_join(clients[i].thread, NULL);

    free(clients);
    free(data);
    return EXIT_SUCCESS;
}
