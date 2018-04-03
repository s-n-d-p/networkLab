#include<stdio.h>
#include<unistd.h> //Provides access to POSIX OS API

int main(){
	pid_t childPid = fork();
	if(childPid == 0)
	printf("Hello\n");
}