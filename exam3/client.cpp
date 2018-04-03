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


	// send(sockfd,argv[2],(size_t)sizeof(argv[2]),0);	//sending port number

	recv(sockfd,buffer,sizeof(buffer),0); //asking for username
	printf("%s",buffer);

	scanf("%[^\n]",buffer);
	fgetc(stdin);
	send(sockfd,buffer,(size_t)sizeof(buffer),0); //sending username

	recv(sockfd,buffer,sizeof(buffer),0); //response from server
	printf("%s\n",buffer);


	if(buffer[0] == 'U'){
		//should try again, with new username
		return 0;
	}
	else{

		//already in, do whatever you want
		while(1){

			recv(sockfd,buffer,sizeof(buffer),0);
			printf("%s\n",buffer);

			// cout << endl;

			// fflush(stdout);

			scanf("%[^\n]",buffer);
			fgetc(stdin);
			send(sockfd,buffer,(size_t)sizeof(buffer),0); //command sent
			

			recv(sockfd,buffer,sizeof(buffer),0); //first response to the command
			// printf("%s\n",buffer);


			if(strcasecmp(buffer,"loop") == 0){
				while(1){
					recv(sockfd,buffer,sizeof(buffer),0);
					if(strcasecmp(buffer,"endloop") == 0) break;
					printf("Username: %s\n",buffer);
					recv(sockfd,buffer,sizeof(buffer),0);
					printf("Port Number: %s\n",buffer);
					recv(sockfd,buffer,sizeof(buffer),0);
					printf("Total Bytes: %s\n\n",buffer);
					// fflush(stdout);
				}
			}
			else if(buffer[0] == '~'){
				recv(sockfd,buffer,sizeof(buffer),0);
				printf("Username: %s\n",buffer);
				recv(sockfd,buffer,sizeof(buffer),0);
				printf("Port Number: %s\n",buffer);	
				recv(sockfd,buffer,sizeof(buffer),0);
				printf("Total Bytes: %s\n\n",buffer);			
			}
			else if(buffer[0] == '^'){
				return 0;
			}
			else if(buffer[0] == 'U'){
				// recv(sockfd,buffer,sizeof(buffer),0);
				printf("%s\n",buffer);
			}
		}

	}



	// while(1){
	// 	scanf("%[^\n]",buffer);
	// 	fgetc(stdin);
	// 	send(sockfd,buffer,(size_t)sizeof(buffer),0);	
	// 	recv(sockfd,buffer,sizeof(buffer),0);
	// 	printf("%s\n",buffer);

	// }
	return 0;
}
