#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ncurses.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "headers.h"

#define ERRO fprintf(stderr, "%s error: %s", clientError, strerror(errno));
#define PORT 4444
#define BUFSIZE 2048

int clientSocket = 0;
int connectFd;
char clientError[12];

char serverBuffer[BUFSIZE];
char clientBuffer[BUFSIZE];
char buffer[BUFSIZE];

void serverConfig();
int parseApplication(char *str);

struct sockaddr_in serverAddr, clientAddr;
socklen_t serverSize = 0;

int main(){
	serverConfig();
	serverSize = sizeof(serverAddr);
	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	connectFd = connect(clientSocket, (struct sockaddr*)&serverAddr, serverSize);
	if(connectFd == -1){
		strncpy(clientError, "Connect", 7);
		ERRO;
		return EXIT_FAILURE;
	}
	
	while(1){
		memset(serverBuffer, 0, sizeof(serverBuffer));
		memset(buffer, 0, sizeof(buffer));
		if(recv(clientSocket, &serverBuffer, sizeof(serverBuffer), 0) == -1){
			strncpy(clientError, "RECV", 7);
			ERRO
			exit(1);
		}
		printf("%s", serverBuffer);
		parseApplication(serverBuffer);
		fgets(buffer, sizeof(buffer), stdin);
		buffer[strcspn(buffer, "\n")] = 0;
		send(clientSocket, buffer, strlen(buffer), 0);
	}
	close(connectFd);
			
}

void serverConfig(){
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_family = AF_INET;
}

int parseApplication(char *str){
	if(strstr(str, "LIVE CHAT")){
		livechat(clientSocket);
	}
}
