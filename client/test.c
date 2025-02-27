#include <stdio.h>
#include <string.h>

char servbuf[] = {"ok monalisa how good is this"};
int main(){
	char input[] = {"how"};
	char *ptr = strstr(servbuf, input);
		int index = ptr - servbuf;
		printf("%d", index);
		printf("%s", &servbuf[index]);

}
