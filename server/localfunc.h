#ifndef LOCALH_
#define LOCALH_

#include <stdbool.h>
#include <netinet/in.h>

typedef unsigned int funcFlags;
#define USEREXISTS (1 << 0)
#define LOGINCHECK (2 << 0)
#define ADDACCOUNT (3 << 0)
#define UPDATEACCOUNT (4 << 0)
#define USERINFO (5 << 0)

typedef unsigned int statusFlags;
#define ONLINE (1 << 0)
#define AFK (2 << 0)
#define OFFLINE (3 << 0)

#define COLOR_RED  	1
#define COLOR_GREEN 	2
#define COLOR_YELLOW	3
#define COLOR_BLUE  	4


#define MAXCLIENTS 10

typedef struct client_t{
	int socket;
	int status;
	char nickname[30];
	struct sockaddr_in address;
}client_t;

bool databaseQuery(char *dbUsername, char *dbPassword, funcFlags flags);
char *md5Check(char *md5Password);
bool mainApplication(char *username, int clientFd);

extern char* serverIp;
extern char* peerIp;
extern char usernameLogin[30];

#endif
