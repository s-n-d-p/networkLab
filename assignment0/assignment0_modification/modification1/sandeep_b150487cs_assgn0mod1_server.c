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

int num1,num2;
void getNumbers(char *buffer){
	int i = 0; num1 = num2 = 0;
	while(!(buffer[i] >= '0' && buffer[i] <= '9')) i++;
	num1 = buffer[i]-'0'; i += 1;
	while(!(buffer[i] >= '0' && buffer[i] <= '9')) i++;
	num2 = buffer[i]-'0';
}

void getnumber(char *buffer){
	int i = 0; num1 = 0;
	while(!(buffer[i] >= '0' && buffer[i] <= '9')) i++;
	while(buffer[i] >= '0' && buffer[i] <= '9'){num1 = num1*10 + buffer[i]-'0'; i++;}

}

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
	printf("Listening\n");
	peer_addr_size = sizeof(struct sockaddr_storage);
	newfd = accept(sockfd,(struct sockaddr *)&peer_addr,&peer_addr_size);
	
	while(1){
		recv(newfd,buffer,sizeof(buffer),0);
		// printf("Data received %s\n",buffer);
		double ans;

		if(buffer[0] == '+'){
			getNumbers(buffer);
			ans = num1 + num2;
		}
		else if(buffer[0] == '-'){
			getNumbers(buffer);
			ans = num1 - num2;
			
		}	
		else if(buffer[0] == '*'){
			getNumbers(buffer);
			// printf("%d\n",num1*num2);
			ans = num1*num2;
			
		}	
		else if(buffer[0] == '/'){
			getNumbers(buffer);
			// printf("%f\n",1.0*num1/num2);
			ans = (num1*1.0)/num2;
			
		}
		else if(buffer[0] == 's'){
			getnumber(buffer);
			// printf("%d\n",num1);
			ans = sin(1.0*num1);
		}
		else if(buffer[0] == 'c'){
			getnumber(buffer);
			// printf("%d\n",num1);
			ans = cos(1.0*num1);
		}
		else{
			break;	
		}
		memset(buffer,0,sizeof(buffer));
		gcvt(ans,10,buffer);
		send(newfd,buffer,(size_t)sizeof(buffer),0);	

	}

	

	return 0;
}
