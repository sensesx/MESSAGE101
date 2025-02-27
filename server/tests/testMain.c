<stdio.h>
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

// Function to send a message to all connected clients except the sender.
void broadcast(char *message, int sender_sock) {
    pthread_mutex_lock(&clientsMutex); // Lock the mutex to prevent race conditions

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] && clients[i]->socket != sender_sock) { // Ensure valid client and not the sender
            send(clients[i]->socket, message, strlen(message), 0); // Send the message
        }
    }

    pthread_mutex_unlock(&clientsMutex); // Unlock the mutex
}

/**
 * Function to handle communication with a connected client.
 */
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

    // If the client disconnects, remove it from the client list
    close(cli->socket); // Close the socket
    pthread_mutex_lock(&clientsMutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] == cli) {
            clients[i] = NULL; // Remove client from list
            break;
        }
    }
    pthread_mutex_unlock(&clientsMutex);

    free(cli); // Free allocated memory for the client
    pthread_exit(NULL); // Exit the thread
}

/**
 * Main function: Starts the chat server and listens for incoming client connections.
 */
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

    // Main loop: Accept clients and create a thread for each
    while (1) {
        // Accept a new client connection
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_size);
        // Allocate memory for new client
        client_t *cli = (client_t *)malloc(sizeof(client_t));
        cli->socket = client_sock;
        cli->address = client_addr;

        // Add client to the list
        pthread_mutex_lock(&clientsMutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (!clients[i]) { // Find an empty slot
                clients[i] = cli;
                break;
            }
        }
        pthread_mutex_unlock(&clientsMutex);

        // Create a new thread to handle this client
        pthread_t thread;
        pthread_create(&thread, NULL, handle_client, (void *)cli);
        pthread_detach(thread); // Automatically free thread resources when it finishes
    }

    return 0;
}
