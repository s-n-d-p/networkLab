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
#define NO_OF_QUES 3
#define QUES_SIZE 100
#define ANS_SIZE 100

char buffer[BUFFER_SIZE];
char questions[NO_OF_QUES][QUES_SIZE];
char ans[NO_OF_QUES][ANS_SIZE];

void initQues(){
	strcpy(questions[0],"How long is one month?");
	strcpy(questions[1],"Which is the only even prime?");
	strcpy(questions[2],"Who is the CEO of Google?");
	strcpy(ans[0],"30 days");
	strcpy(ans[1],"2");
	strcpy(ans[2],"Sundar Pichai");
}
int terminate = 0;
void ansQuery(){
	int n = strlen(buffer), f=0;
	if(!strcasecmp(buffer,"Bye")){terminate = 1;return;}
	if(buffer[n-1] != '?') strcpy(buffer,"Wrong question format");
	else{
		for(int i = 0; i < NO_OF_QUES; i++){
			if(!strcasecmp(buffer,questions[i])){
				f = 1; strcpy(buffer,ans[i]); break;
			}
		}
		if(!f){strcpy(buffer,"Unknown question");}
	}	
}

int main(int argc,char *argv[]){

	initQues();
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
	printf("%s\n",buffer);
	strcpy(buffer,"Hi Client");
	send(newfd,buffer,(size_t)sizeof(buffer),0);


	while(!terminate){
		recv(newfd,buffer,sizeof(buffer),0);
		printf("Query recieved\n");
		ansQuery();
		send(newfd,buffer,(size_t)sizeof(buffer),0);		
	}
	printf("Exiting\n");
	return 0;
}
