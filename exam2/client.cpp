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
	if(signo == SIGINT) printf("\nUser : Exiting"),exit(1);
}

int main(int argc,char *argv[]){

	if(argc != 3){
		printf("Usage: ./user <destination_ip> <port_number>\n");
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
		fprintf(stderr, "User, getaddrinfo: %s\n",gai_strerror(status));
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



	recv(sockfd,buffer,sizeof(buffer),0);
	printf("%s",buffer);
	scanf("%[^\n]",buffer);
	fgetc(stdin);
	send(sockfd,buffer,(size_t)sizeof(buffer),0);
	recv(sockfd,buffer,sizeof(buffer),0);
	printf("%s",buffer);
	scanf("%[^\n]",buffer);
	fgetc(stdin);
	send(sockfd,buffer,(size_t)sizeof(buffer),0);
	
	recv(sockfd,buffer,sizeof(buffer),0);
	printf("%s\n",buffer);

	int flag = 0;
	if(strcasecmp(buffer,"Wrong password, try again later") == 0){
		return 0;
	}
	else{
		while(flag != 1){
			recv(sockfd,buffer,sizeof(buffer),0);
			printf("%s",buffer);
			recv(sockfd,buffer,sizeof(buffer),0);
			printf("%s",buffer);
			recv(sockfd,buffer,sizeof(buffer),0);
			printf("%s",buffer);

			printf("Choice: ");

			memset(buffer,'\0',BUFFER_SIZE);
			scanf("%[^\n]",buffer);
			fgetc(stdin);
			send(sockfd,buffer,(size_t)sizeof(buffer),0);

			if(buffer[0] == '1'){
				printf("\n");
				while(1){
					recv(sockfd,buffer,sizeof(buffer),0);
					if(strcasecmp(buffer,"EOM") == 0){
						break;
					}
					else{
						printf("From : %s\n",buffer);
						recv(sockfd,buffer,sizeof(buffer),0);
						printf("Message : %s\n",buffer);

						printf("..................................................\n");
					}
				}
			}
			else if(buffer[0] == '2'){
				recv(sockfd,buffer,sizeof(buffer),0);
				printf("%s",buffer);
				scanf("%[^\n]",buffer);
				fgetc(stdin);
				send(sockfd,buffer,(size_t)sizeof(buffer),0);
				
				recv(sockfd,buffer,sizeof(buffer),0);
				printf("%s",buffer);
				if(strcasecmp(buffer,"Invalid Email") == 0){
					flag = 1;
				}
				else{
					scanf("%[^\n]",buffer);
					fgetc(stdin);
					send(sockfd,buffer,(size_t)sizeof(buffer),0);

				}

			}
			else{
				//logout the user

				flag = 1;
			}	
		}	
	}


	// while(1){
		


			
	// 	// scanf("%[^\n]",buffer);
	// 	// fgetc(stdin);
	// 	// send(sockfd,buffer,(size_t)sizeof(buffer),0);	
	// 	// recv(sockfd,buffer,sizeof(buffer),0);
	// 	// printf("%s\n",buffer);

	// }
	return 0;
}
