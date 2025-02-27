#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <cjson/cJSON.h>
#include <sys/socket.h>
#include "localfunc.h"

extern client_t *clients[10];
char LINES[256];
char userconfigJson[256];
char filename[] = {"./config/userconfig.json"};
FILE *fp;

char config[4][10] = {"RED", "GREEN", "YELLOW", "BLUE"};
char buffer[1024];

void *broadcastMessage(char *buffer, int senderSock){
	char bufferToSend[1024];
	for(int i = 0; i < MAXCLIENTS; i++){
		if((clients[i] && clients[i]->socket != senderSock) && (clients[i]->status == ONLINE)){
			snprintf(bufferToSend, sizeof(bufferToSend), "\n%s", buffer);
			printf("\n\nBUFFER TO SEND: %s", bufferToSend);
			send(clients[i]->socket, bufferToSend, strlen(bufferToSend), 0);
		}
	}
}


int parseJson(char *string){
	int count =0;
	fp = fopen(filename, "r");
	char *usercolor = NULL;
	cJSON *json = cJSON_Parse(string);
	if(!json){
		printf("Error parsing JSON\n");
	}
	cJSON *color = cJSON_GetObjectItemCaseSensitive(json, "chatcolor");
	usercolor = color->valuestring;
	for(int i = 0; i < 3; i++){
		if(strstr(usercolor, config[i])){
			break;	
		}
		count++;
	}
	cJSON_Delete(color);
	return count;
}

bool mainApplication(char *username, int clientFd){
	char serverBuffer[256];
	char userBuffer[256];
	char userMessage[1024];
	char bracketUsername[30];
	int colorNumber;
	size_t bytesRead = 0;

	if(access(filename, F_OK) == 0){
		fp = fopen(filename, "r");
		send(clientFd, "[+] User configurations found.", 30, 0);
		send(clientFd, "\n[+] Loading user configurations for this computer.", 50, 0);
		while(fgets(LINES, sizeof(LINES), fp)){
			strcat(userconfigJson, LINES);
			}
		}
	else{
		send(clientFd, "\n[-] We did not find user configurations on your computer.\n[-] Would you like to configure it now? (Y/N)\nR: ", 108, 0);
		while(1){
			recv(clientFd, userBuffer, strlen(userBuffer), 0);
			char option;
			strcat(&option, userBuffer);
			switch(option){
				case 'Y':
					send(clientFd, "\n[+] Creating file", 19, 0);
					fp = fopen(filename, "w");
					break;
				case 'N':
					send(clientFd, "\n[!] Using default configuration", 32, 0);
					break;
				default:
					send(clientFd, "\nOnly Y or N\nR:", 13, 0);
			}
		}
	}
	if(fp != NULL){
		fprintf(stdout, "\n**** Closing configuration file");
		fclose(fp);
	}

	while(1){
		snprintf(serverBuffer, sizeof(serverBuffer), "\n\n=== Welcome %s ===\n\nChoose any of the options below\n\n 1. Live chat\n \
2. Private Messages\n \
3. My information\n \
4. Add a friend.\n \
5. Leave\n\nR: ", username);
		send(clientFd, serverBuffer, strlen(serverBuffer), 0);
		recv(clientFd, userBuffer, strlen(userBuffer), 0);
		switch(atoi(userBuffer)){
			case 1:
				memset(serverBuffer, 0, sizeof(serverBuffer));
				snprintf(serverBuffer, sizeof(serverBuffer), "\n\nLIVE CHAT - MESSAGE101 - TALK TO PEOPLE\n\n-> : ");
				send(clientFd, serverBuffer, sizeof(serverBuffer), 0);
				//fcntl(clientFd, F_SETFL, O_NONBLOCK);
				while(1){
					bytesRead = recv(clientFd, userMessage, sizeof(userMessage), 0);
					userMessage[bytesRead]	= '\0';
					userMessage[strcspn(userMessage, "\n")] = 0;
				 //	send(clientFd, bracketUsername, strlen(bracketUsername), 0);
					broadcastMessage(userMessage, clientFd);
				}
				break;
			case 2: 
				memset(serverBuffer, 0, sizeof(serverBuffer));
				snprintf(serverBuffer, sizeof(serverBuffer), "You have got 0 messages"); // IMPLEMENT
				break;
			case 3:
				
			case 4:
				memset(serverBuffer, 0, sizeof(serverBuffer));
				snprintf(serverBuffer, sizeof(serverBuffer), "\n\nID code or nickname of the user: ");
				break;
			default:
				printf("\n\nOption does not exists. ");
				break;
		}
	}
	return true;
}

