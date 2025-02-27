#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <mysql/mysql.h>
#include "localfunc.h"

typedef unsigned int funcFlags;
#define USEREXISTS (1 << 0)
#define LOGINCHECK (2 << 0)
#define ADDACCOUNT (3 << 0)
#define UPDATEACCOUNT (4 << 0)
#define USERINFO (5 << 0)

MYSQL *mysql;
MYSQL_RES *result;
int fieldNumbers = 0;
MYSQL_ROW rows = 0;
char sqlQuery[256] = {0};

bool databaseQuery(char *username, char *password, funcFlags flags){ 
	memset(sqlQuery, 0, strlen(sqlQuery));
	username[strcspn(username, "\n")] = 0;
	mysql = mysql_init(NULL);
	if(mysql == NULL){
		printf("\n\n**** -> Could not initialize mysql. ");
		return EXIT_FAILURE;
	}else{
		if(!mysql_real_connect(mysql, "127.0.0.1", "morph", "test", "serverInfo", 0, 0, 0)){
			printf("\n\n**** -> Could not connect to database. Exiting. ");
			return EXIT_FAILURE;
		}else{
			printf("\n**** READY TO QUERY DATABASE");
		}
	}

	fflush(stdout);

	if(flags == USERINFO){
		memset(sqlQuery, 0, sizeof(sqlQuery));
		//fprintf(stdout, "\n**** -> [%s] Checking if user exists", serverIp);
		snprintf(sqlQuery, sizeof(sqlQuery), "select * from userInfo where username='%s';", username);
	
		if(!mysql_real_query(mysql, sqlQuery, strlen(sqlQuery))){
			//fprintf(stdout, "\n**** -> [%s] QUERY: %s", serverIp, sqlQuery);
	//		fprintf(stdout, "\n**** -> [%s] Checking username %s on the database", serverIp, username);
		}else{
			printf("\n **** There was an error trying to query database: ");
			printf("%s", mysql_error(mysql));
		}
		result = mysql_store_result(mysql);
		if(mysql_fetch_row(result)){
			//fprintf(stdout, "\n**** -> [%s] Username is being used. User should try again. ", serverIp);
			return true;
		}else{
			//fprintf(stdout, "\n**** -> [%s] Username %s is not being used.", serverIp, username);
			return false;
		}
		flags &= ~USERINFO;
		mysql_close(mysql);
		mysql_library_end();

	}
	
	if(flags == USEREXISTS){
		memset(sqlQuery, 0, strlen(sqlQuery));
		//fprintf(stdout, "\n**** -> [%s] Checking if user exists", serverIp);
		snprintf(sqlQuery, sizeof(sqlQuery), "select * from userInfo where username='%s';", username);
	
		if(!mysql_real_query(mysql, sqlQuery, strlen(sqlQuery))){
			//fprintf(stdout, "\n**** -> [%s] QUERY: %s", serverIp, sqlQuery);
			//fprintf(stdout, "\n**** -> [%s] Checking username %s on the database", serverIp, username);
		}else{
			printf("\n **** There was an error trying to query database: ");
			printf("%s", mysql_error(mysql));
		}
		result = mysql_store_result(mysql);
		if(mysql_fetch_row(result)){
		//	fprintf(stdout, "\n**** -> [%s] Username is being used. User should try again. ", serverIp);
			return true;
		}else{
			//fprintf(stdout, "\n**** -> [%s] Username %s is not being used.", serverIp, username);
			return false;
		}
		flags &= ~USEREXISTS;
		mysql_close(mysql);
		mysql_library_end();
	}
	if(flags == LOGINCHECK){
		memset(sqlQuery, 0, strlen(sqlQuery));
		//fprintf(stdout, "\n**** -> [%s] Logging user in. Checking if credentials match.", serverIp);
		snprintf(sqlQuery, sizeof(sqlQuery), "select * from userInfo where username='%s' and password='%s';", username, password);
		printf("\n**** SQL Query: %s", sqlQuery);
		if(!mysql_real_query(mysql, sqlQuery, strlen(sqlQuery))){
//			fprintf(stdout, "\n**** -> [%s] QUERY: %s", serverIp, sqlQuery);
//			fprintf(stdout, "\n**** -> [%s] Checking account %s:%s on the database", serverIp, username, password);
		}else{
			printf("\n**** There was an error trying to query database: ");
			printf("%s", mysql_error(mysql));
		}
		result = mysql_store_result(mysql);
		if(mysql_fetch_row(result)){
			printf("\n**** Credentials match. User is logged in.");
		//	fprintf(stdout, "\n**** -> [%s] Credentials Match. User is being connected. ", serverIp);
			flags &= ~LOGINCHECK;
			mysql_close(mysql);
			mysql_library_end();
			return true;
		}else{
			printf("\n**** Credentials do not match. User must try again.");
		//	fprintf(stdout, "\n**** -> [%s] Credentials don't match. User needs to try again. ", serverIp);
			flags &= ~LOGINCHECK;
			mysql_close(mysql);
			mysql_library_end();
			return false;
		}
	}

	if(flags == ADDACCOUNT){
		memset(sqlQuery, 0, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "insert into userInfo (username, password) VALUES('%s','%s');", username, password);
	}
}



