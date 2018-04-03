#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include<signal.h>

#define BUFFER_SIZE 1024
char buffer[BUFFER_SIZE];


void sig_handler(int signo){
	if(signo == SIGINT) printf("\nClient : Exiting"),exit(1);
}

void chat(int sockfd){
	char USER[BUFFER_SIZE];
	init_chat:

	printf("Username: ");

	scanf("%[^\n]",buffer); /*username*/ fgetc(stdin);
	send(sockfd,buffer,(size_t)sizeof(buffer),0);

	strcpy(USER,buffer);
	int temp = strlen(USER);
	USER[temp] = ':';
	USER[temp + 1] = ' ';
	USER[temp + 2] = '\0';

	printf("Password: ");

	scanf("%[^\n]",buffer); /*password*/ fgetc(stdin);
	send(sockfd,buffer,(size_t)sizeof(buffer),0);

	recv(sockfd,buffer,sizeof(buffer),0); 
	if(buffer[0] == 'R' || buffer[0] == 'L'){
		printf("%s",buffer);
		while(1){
			recv(sockfd,buffer,sizeof(buffer),0); 
			if(strcasecmp(buffer,"end") == 0) break;
			else {
				printf("%s\n",buffer);
			}
		}

		printf("\n");

		pid_t pid;


		if((pid = fork()) == 0){
			while(1){
				recv(sockfd,buffer,sizeof(buffer),0); 
				printf("%s\n",buffer);
				recv(sockfd,buffer,sizeof(buffer),0); 
				printf("%s\n",buffer);
			}

		}
		else{

			while(1){
				scanf("%[^\n]",buffer); /*username*/ fgetc(stdin);
				if(strcasecmp(buffer,"logout") == 0){
					kill(pid,SIGKILL);
					send(sockfd,buffer,(size_t)sizeof(buffer),0);
					exit(0);
				}
				send(sockfd,buffer,(size_t)sizeof(buffer),0);

				// send(sockfd,USER,(size_t)sizeof(USER),0);

				strcpy(buffer,USER);
				
				scanf("%[^\n]",buffer+strlen(buffer)); /*password*/ fgetc(stdin);
				send(sockfd,buffer,(size_t)sizeof(buffer),0);


			}

		}


	}
	else{
		goto init_chat;
	}
}

int main(int argc,char *argv[]){

	if(argc != 3){
		printf("Usage: ./client <destination_ip> <port_number>\n");
		exit(1);
	}

	signal(SIGINT,sig_handler);
	int sockfd,status,newfd;
	struct addrinfo hints, *res,*p;
	memset(&hints,0,sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	status = getaddrinfo(argv[1],argv[2],&hints,&res);
	if(status != 0){
		fprintf(stderr, "Client, getaddrinfo: %s\n",gai_strerror(status));
		exit(EXIT_FAILURE);
	}
	for(p = res; p != NULL; p = p->ai_next){
		sockfd = socket(p->ai_family,p->ai_socktype,p->ai_protocol);
		if(sockfd == -1) continue;
		if(connect(sockfd,p->ai_addr,p->ai_addrlen) == 0){
			break;
		}
		close(sockfd);
	}
	freeaddrinfo(res);
	if(p == NULL){
		fprintf(stderr, "Could not connect\n");
		exit(EXIT_FAILURE);
	}

	while(1){
		chat(sockfd);
	}
	return 0;
}
