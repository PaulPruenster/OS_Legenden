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
    // first message the server receives is the name of the client
    if (!recv(c->connfd, buff, MAX, 0))
    {
        atomic_fetch_sub(data->server_data->clients_connected, 1);
        close(c->connfd);
        c->isAlive = false;
        free(arg);
        free(buff);
        return NULL;
    }
    // set name 
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
            atomic_fetch_sub(data->server_data->clients_connected, 1);
            c->isAlive = false;
            fflush(stdout);
            pthread_cancel(listenthr);
            printf("%s disconnected\n", c->name);
            printf("Shutting down.\n");
            close(c->connfd);
            free(arg);
            free(buff);
            return NULL;
        }

        char *message = malloc(sizeof(char) * MAX);
        char buffcpy[MAX] = {0};
        strcpy(buffcpy, buff);

        char *word;
        char *raw_message;
        // get first word
        word = strtok(buffcpy, " ");

        // Check if user wants to whisper to someone
        if (!strcmp(word, "/w"))
        {
            // mutex to prevent user to disconnect 
            // get name
            word = strtok(NULL, " ");

            // https://stackoverflow.com/questions/19724450/c-language-how-to-get-the-remaining-string-after-using-strtok-once
            raw_message = word + strlen(word) + 1;

            // dont send the message if something is missing
            if (word == NULL || raw_message == NULL)
                continue;

            else
            {
                sprintf(message, "%s (whispers): %s", c->name, raw_message);
                size_t max_clients = *data->server_data->clients_counter;
                size_t i = 0;
                while (i < max_clients)
                {
                    // send if client is alive and name is the same as given by user
                    if (data->server_data->clients[i].isAlive && !strcmp(data->server_data->clients[i].name, word))
                    {
                        send(data->server_data->clients[i].connfd, message, strlen(message), 0);
                        break;
                    }
                    i++;
                }
            }
        }
        // otherwise message will get send to everyone
        else
        {
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
        }
        // mutex unlock
        // print the message received from client
        printf("%s: %s", c->name, buff);
        fflush(stdout);
        free(message);
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

        data->clients[i].connfd = accept(sockfd, (struct sockaddr *)&client, &peer_addr_size);
        if (data->clients[i].connfd < 0)
        {
            perror("server accept failed...\n");
            continue;
        }
        data->clients[i].isAlive = true;
        atomic_fetch_add(data->clients_connected, 1);
        atomic_fetch_add(data->clients_counter, 1);

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
    if (port < 0 || port > 65536)
    {
        fprintf(stderr, "Port: %d does not exist!", port);
        return EXIT_FAILURE;
    }
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
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
    {
        perror("Binding failed...\n");
        return EXIT_FAILURE;
    }
    printf("Socket successfully bound..\n");
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
    {
        pthread_join(clients[i].thread, NULL);
    }

    free(clients);
    free(data);
    return EXIT_SUCCESS;
}
