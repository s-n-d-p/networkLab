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

#define BUFFER_SIZE 1024
char buffer[BUFFER_SIZE];
int terminate = 0;

int main(int argc,char *argv[]){
	if(argc != 3){
		printf("Usage: ./client <destination_ip> <port_number>\n");
		exit(1);
	}
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

	// Hi message 
	printf("Client : Connection establised\n");
	strcpy(buffer,"Hi Server");	
	send(sockfd,buffer,(size_t)sizeof(buffer),0);	
	recv(sockfd,buffer,sizeof(buffer),0);
	printf("%s\n",buffer);
	// Hi message exhanged

	while(!terminate){
		scanf("%[^\n]",buffer); fgetc(stdin);
		if(!strcasecmp(buffer,"quit")){strcpy(buffer,"Bye"); terminate = 1;}
		send(sockfd,buffer,(size_t)sizeof(buffer),0);	
	 	recv(sockfd,buffer,sizeof(buffer),0);
	 	printf("%s\n",buffer);
	}
	return 0;
}
