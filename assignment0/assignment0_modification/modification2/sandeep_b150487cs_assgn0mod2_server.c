#include<stdio.h>
#include<sys/types.h>
#include <sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<math.h>

#define BUFFER_SIZE 1024
#define BACKLOG 5

char buffer[BUFFER_SIZE];

int main(int argc,char *argv[]){

	if(argc != 2){
			printf("Usage: ./server <port_number>\n");
			exit(1);
		}

	int sockfd,status,newfd;
	struct addrinfo hints, *res,*p;
	struct sockaddr_storage peer_addr;
	socklen_t peer_addr_size;
	memset(&hints,0,sizeof hints);
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	status = getaddrinfo(NULL,argv[1],&hints,&res);
	if(status != 0){
		fprintf(stderr, "Server, getaddrinfo: %s\n",gai_strerror(status));
		exit(EXIT_FAILURE);
	}
	for(p = res; p != NULL; p = p->ai_next){
		sockfd = socket(p->ai_family,p->ai_socktype,p->ai_protocol);
		if(sockfd == -1) continue;
		if(bind(sockfd,p->ai_addr,p->ai_addrlen) == 0) break;
		close(sockfd);
	}
	freeaddrinfo(res);
	if(p == NULL){
		fprintf(stderr, "Could not bind\n");
		exit(EXIT_FAILURE);
	}
	if(listen(sockfd,BACKLOG) == -1){
		perror("Server, listen");
		exit(EXIT_FAILURE);
	}
	printf("Listening....\n");
	peer_addr_size = sizeof(struct sockaddr_storage);
	newfd = accept(sockfd,(struct sockaddr *)&peer_addr,&peer_addr_size);
	
	recv(newfd,buffer,sizeof(buffer),0);
	if(access(buffer,F_OK) != 0){
		strcpy(buffer,"File not found");
		send(newfd,buffer,(size_t)sizeof(buffer),0);	
		strcpy(buffer,"exit");
		send(newfd,buffer,(size_t)sizeof(buffer),0);		
		exit(EXIT_FAILURE);
	}
	char c;
	FILE *fp = fopen(buffer,"r");
	int i = 0;
	while((c = fgetc(fp)) != EOF){
		buffer[i++] = c;
		if(i == BUFFER_SIZE){
			send(newfd,buffer,(size_t)sizeof(buffer),0);		
			i = 0;
		}
	}

	if(i != 0){
		send(newfd,buffer,(size_t)sizeof(buffer),0);		
	}

	strcpy(buffer,"exit");
	send(newfd,buffer,(size_t)sizeof(buffer),0);		

	
	return 0;
}
