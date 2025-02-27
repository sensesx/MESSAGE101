/* Chat Application - MESSAGE 101
* 
* Application made by sensesx
* 
*/


/* TODO: Multithreading so more clients can connect*/
/* MYSQL must be running because it checks if user exists */
/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include "localfunc.h"
#include <pthread.h>

#define CLIENTSMAX 10

// Sizes
ssize_t serverInfoLen = 0;

// Functions
void broadcastMessage(char *buffer, int pClientFd);

typedef struct cclient_t{
	int clientFd;
	struct sockaddr_in clientAddr;
}client_t;

client_t *clients[CLIENTSMAX];

struct sockaddr_in serverInfo, clientInfo;

// Variables
FILE *serverLog;
char *PWD;
char buffer[1024];
int serverSocket, clientSocket;


pthread_mutex_t clientsMutex = PTHREAD_MUTEX_INITIALIZER; // Creating var for mutex and different threads manipulation

void *handleConnection(void *arg){
	client_t *argument = (client_t *)arg;
	recv(argument->clientFd, buffer, sizeof(buffer), 0);
	broadcastMessage(buffer, argument->clientFd);

}

void broadcastMessage(char *buffer, int pClientFd){
	for(int i = 0; i < CLIENTSMAX; i++){
		send(clients[i]->clientFd, buffer, sizeof(buffer), 0);
	}
}

int main(){
	PWD = getenv("PWD");
	strcat(PWD, "/logs/message101.logs");
	serverLog = fopen(PWD, "w+");
	serverInfo.sin_addr.s_addr = inet_addr("172.24.239.227");
	serverInfo.sin_port = htons(4444);
	serverInfo.sin_family = AF_INET;
	serverInfoLen = sizeof(serverInfo);
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
//	inet_ntoa(serverInfo.sin_addr = inet_ntoa(serverInfo.sin_addr);
//
	printf("hi");
	if(bind(serverSocket, (struct sockaddr*)&serverInfo, serverInfoLen) == -1){
		fprintf(stderr, "**** -> [%s] Bind error: %s",inet_ntoa(serverInfo.sin_addr), strerror(errno));
		return EXIT_FAILURE;
	}
	fprintf(stdout, "\n\n**** -> [%s] Initializing server on port (%d)", inet_ntoa(serverInfo.sin_addr), ntohs(serverInfo.sin_port));
	fprintf(stdout, "\n**** -> [%s] Waiting for connection\n", inet_ntoa(serverInfo.sin_addr));
	if(listen(serverSocket, 1) == -1){
		fprintf(stderr, "**** -> Listen error: %s", strerror(errno));
		return EXIT_FAILURE;
	}

	socklen_t addrCliSize = sizeof(struct sockaddr_in);
	printf("hi");
	while(1){
		clientSocket = accept(serverSocket, (struct sockaddr*)&clientInfo, &addrCliSize);
		client_t *client = (client_t *)malloc(sizeof(client_t));
		client->clientFd = clientSocket; // FILE DESCRIPTOR
		client->clientAddr = clientInfo; // EQUALS SOCKADDR_IN
						 
		pthread_mutex_lock(&clientsMutex); // LOCK DE MUTUAL EXECUTION
		for(int i  = 0; i < CLIENTSMAX; i++){ // Arrumando espaço no buffer de client_t para guardar FD
			if(!clients[i]){
				clients[i] = client;
				break;
			}
		}
		pthread_mutex_unlock(&clientsMutex); // LOCK DE MUTUAL EXECUTION
		pthread_t thread;
		pthread_create(&thread, NULL, handleConnection, (void *)client);
		pthread_detach(thread);
	}
}


*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>       // For close()
#include <pthread.h>      // For pthreads
#include <arpa/inet.h>    // For sockets

#define PORT 8080         // Server listening port
#define MAX_CLIENTS 10    // Maximum number of clients that can connect
#define BUFFER_SIZE 1024  // Size of the message buffer

typedef struct {
    int socket;                // Client socket descriptor
    struct sockaddr_in address; // Client address
} client_t;
client_t *clients[MAX_CLIENTS];
pthread_mutex_t clientsMutex = PTHREAD_MUTEX_INITIALIZER;

void broadcast(char *message, int sender_sock) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
	if (clients[i] && clients[i]->socket != sender_sock) { // Ensure valid client and not the sender
            send(clients[i]->socket, message, strlen(message), 0); // Send the message
        }
    }
}

void *handle_client(void *arg) {
    client_t *cli = (client_t *)arg;  // Convert argument to client_t pointer
    char buffer[BUFFER_SIZE];
    int bytes_read;

    while ((bytes_read = recv(cli->socket, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[bytes_read] = '\0'; // Null-terminate the message
        printf("Client %d: %s", cli->socket, buffer); // Print the received message

        // Broadcast the received message to other clients
        broadcast(buffer, cli->socket);
    }
}

int main() {
	int server_sock, client_sock;
	struct sockaddr_in server_addr, client_addr;
	socklen_t addr_size = sizeof(struct sockaddr_in);
	server_sock = socket(AF_INET, SOCK_STREAM, 0);
	server_addr.sin_family = AF_INET;         // IPv4
	server_addr.sin_addr.s_addr = INADDR_ANY; // Accept connections on any network interface
	server_addr.sin_port = htons(PORT);       // Convert port to network byte order
	bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
	listen(server_sock, MAX_CLIENTS);
	printf("Server listening on port %d...\n", PORT);
    while (1) {
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_size);
        client_t *cli = (client_t *)malloc(sizeof(client_t));
        cli->socket = client_sock;
        cli->address = client_addr;

        pthread_mutex_lock(&clientsMutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (!clients[i]) { // Find an empty slot
                clients[i] = cli;
                break;
            }
        }
        pthread_mutex_unlock(&clientsMutex);

        pthread_t thread;
        pthread_create(&thread, NULL, handle_client, (void *)cli);
        pthread_detach(thread); // Automatically free thread resources when it finishes
    }

    return 0;
}

