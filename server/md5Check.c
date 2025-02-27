#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "localfunc.h"
#include <sys/wait.h>
#include <string.h>

FILE *temporaryFile;

char *md5Check(char *md5Password){ 				// DANGEROUS: gives uses control of a buffer that determines which command
	int fileDescriptor[2];
	char command[50];
	char *ptr;
	ptr = (char *)malloc(sizeof(char *) * 50);
	md5Password[strcspn(md5Password, "\n")] = 0;
	temporaryFile = fopen("temp.txt", "w+");
	fputs(md5Password, temporaryFile);
	fclose(temporaryFile);
	pipe(fileDescriptor);
	int id = fork();
	if(id == 0){
		dup2 (fileDescriptor[1], STDOUT_FILENO);
		execl("/bin/md5sum", "-t", "temp.txt", 0,(char *)0);
		close(fileDescriptor[0]);
		close(fileDescriptor[1]);
  	}else{
		close(fileDescriptor[1]);
		int nbytes = read(fileDescriptor[0], command, sizeof(command));
		close(fileDescriptor[0]);
		wait(NULL);
  	}
	for(int i = 0; i <=32; i++){
		if(command[i] == '\x20'){
			break;
		}
		ptr[i] = command[i];
	}
	return ptr;
}


