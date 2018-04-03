#include<stdio.h>
#include<time.h>

int main(){
	time_t TIME = time(NULL);
	char *m = ctime(&TIME);

	int i = 0;
	while(m[i] != '\n') i++; m[i] = '\0';

	printf("%s\n",m);
}