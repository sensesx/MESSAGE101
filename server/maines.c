#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>       
#include "localfunc.h"
#include <pthread.h>     
#include <arpa/inet.h>    
#include <ncurses.h>

#define PORT 4444
#define BUFFERSIZ 1024 

void *initOption(void *arg);

pthread_mutex_t clientsMutex = PTHREAD_MUTEX_INITIALIZER;
struct client_t *clients[MAXCLIENTS];
char usernameLogin[30];
char passwordLogin[30];

struct sockaddr_in serverAddr, clientAddr;

int main(){
	int server_sock, clientSocket;

	socklen_t addr_size = sizeof(struct sockaddr_in);
	server_sock = socket(AF_INET, SOCK_STREAM, 0);
	serverAddr.sin_family = AF_INET;         // IPv4
	serverAddr.sin_addr.s_addr = INADDR_ANY; // Accept connections on any network interface
	serverAddr.sin_port = htons(PORT);       // Convert port to network byte order
	if(bind(server_sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1){
		fprintf(stderr, "\n**** Binding error. ");
		return EXIT_FAILURE;
	}
	if(listen(server_sock, MAXCLIENTS)){
		fprintf(stderr, "\n**** Listen error. ");
		return EXIT_FAILURE;
	}
	printf("\n**** Server listening on port %d...\n", PORT);

	while (1) {
		clientSocket = accept(server_sock, (struct sockaddr *)&clientAddr, &addr_size);
		if(clientSocket!= -1){
			printf("\n\nConnected");
		}else{
			printf("\n\nTried connecting.");
		}

		fflush(stdout);
		client_t *cli = (client_t *)malloc(sizeof(client_t));  // Allocate memory for new client
		cli->socket = clientSocket;
		cli->address = clientAddr;
		cli->status = OFFLINE;

		pthread_mutex_lock(&clientsMutex);
		for (int i = 0; i < MAXCLIENTS; i++) {
			if (!clients[i]) { // Search for empty slot
				clients[i] = cli;
				break;
				}
		}
		pthread_mutex_unlock(&clientsMutex);
		// Create a new thread to handle this client
		pthread_t thread;
		pthread_create(&thread, NULL, initOption, (void *)cli);
		pthread_detach(thread); // Automatically free thread resources when it finishes
	}
}

void *initOption(void *arg){
	char peerBuffer[BUFFERSIZ];
	char hashedPassword[33];
	char *pHashedPassword;
	bool accountExists = false;
	bool addAccount = false;
	bool loginCheck = false;

	client_t *clientThread = (client_t *)arg; // VOID* CONVERSIONT O CLIENT_T
	char buffer[BUFFERSIZ] = {"\n====================================\n\tWelcome to MESSAGE101 \
	\n\nSelect an option below\n\n 1. Login\n \
2. Create Account\n \
3. Why to use\n \
4. Exit program\n\n R: "};

	while(1){
		send(clientThread->socket, buffer, strlen(buffer) + 1 ,0);
		recv(clientThread->socket, peerBuffer, sizeof(peerBuffer), 0);
		switch(atoi(peerBuffer)){
			case 1:
				memset(usernameLogin, 0, sizeof(usernameLogin));
				memset(passwordLogin, 0, sizeof(passwordLogin));
				send(clientThread->socket, "\nUsername: ", strlen("\nUsername: "), 0);
				recv(clientThread->socket, peerBuffer, sizeof(peerBuffer), 0);
				strncpy(usernameLogin, peerBuffer, strlen(peerBuffer));

				pHashedPassword = (char *)malloc(sizeof(char) * 32);
				send(clientThread->socket, "\nPassword: ", strlen("\nPassword: "), 0);
				recv(clientThread->socket, peerBuffer, sizeof(peerBuffer), 0);
				strncpy(passwordLogin, peerBuffer, strlen(peerBuffer));

				passwordLogin[strcspn(passwordLogin, "\n")] = 0;
				usernameLogin[strcspn(usernameLogin, "\n")] =0;
				pHashedPassword = md5Check(passwordLogin);
				strncpy(hashedPassword, pHashedPassword, 32);

				hashedPassword[32] = '\00';
				loginCheck = databaseQuery(usernameLogin, hashedPassword, LOGINCHECK);
				free(pHashedPassword);
				if(loginCheck){
					send(clientThread->socket, "\n\n=== LOGIN SUCCESSFUL ===\n", 30, 0);
					sleep(1);
					clientThread->status = ONLINE;
					strncpy(clientThread->nickname, usernameLogin, strlen(usernameLogin));
					mainApplication(usernameLogin, clientThread->socket);
				}else{
					send(clientThread->socket, "\n\nLOGIN FAILED\n", 18, 0);
				}
										
				break;
			case 2:
				memset(usernameLogin, 0, sizeof(usernameLogin));
				memset(passwordLogin, 0, sizeof(passwordLogin));
				send(clientThread->socket, "Type a username: ", 18, 0);
				recv(clientThread->socket, peerBuffer, sizeof(peerBuffer), 0);
				strncpy(usernameLogin, peerBuffer, strlen(peerBuffer));
				accountExists = databaseQuery(usernameLogin, 0, USEREXISTS);
				if(accountExists){
					send(clientThread->socket, "\n\nUsername being used. Try another one.\n", 38, 0);
				}else{
					pHashedPassword = (char *)malloc(sizeof(char) * 32);
					send(clientThread->socket, "Password: ", strlen("Username: "), 0);
					recv(clientThread->socket, peerBuffer, sizeof(peerBuffer), 0);
					strncpy(passwordLogin, peerBuffer, strlen(peerBuffer));
					passwordLogin[strcspn(passwordLogin, "\n")] = 0;
					usernameLogin[strcspn(usernameLogin, "\n")] =0;
					pHashedPassword = md5Check(passwordLogin);
					strncpy(hashedPassword, pHashedPassword, 32);
					hashedPassword[32] = '\00';
					addAccount = databaseQuery(usernameLogin, hashedPassword, ADDACCOUNT);
					free(pHashedPassword);
					if(addAccount){
						send(clientThread->socket, "\nAccount creation was successful, now login to your account.\n", 32, 0);
						clientThread->status = true;
						mainApplication(usernameLogin, clientThread->socket);
					}else{
						send(clientThread->socket, "\nServer internal error. Could not create account. Try again.", 32, 0);
					}
				}
				break;
			case 3:
				send(clientThread->socket, "\nThis is an ongoing application. It's made for testing only. Do not use it on public networks because it is unsafe =)\n", 122, 0);
				break;
			case 4:
				send(clientThread->socket, "\nExiting, bye. =)\n", 19, 0);
				close(clientThread->socket);
			default:
				send(clientThread->socket, "\nInvalid option. Try again\n", 29, 0);
				break;
		}
	}
}

