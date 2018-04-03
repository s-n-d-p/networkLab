#include<stdio.h>
#include<sys/types.h>
#include <sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<math.h>
#include<signal.h>
#include<pthread.h>
#include<semaphore.h>

#define BUFFER_SIZE 1024
#define BACKLOG 1

char buffer[BUFFER_SIZE];

int getnumber(char *buffer){
	int i = 0; int num1 = 0;
	while(!(buffer[i] >= '0' && buffer[i] <= '9')) i++;
	while(buffer[i] >= '0' && buffer[i] <= '9'){num1 = num1*10 + buffer[i]-'0'; i++;}
	return num1;
}

void sig_handler(int signo){
	if(signo == SIGINT){
		printf("\nServer : Exiting"), exit(0);
	}
}


struct threadArgs{
	int sockfd, newfd;
};

struct threadArgs *arg;

sem_t ACCESS;


int userID[BUFFER_SIZE];
int userPWD[BUFFER_SIZE];
int loggedIN[BUFFER_SIZE]; //actually stores the socket descriptor of the client

void *threadFunction(void *arg){

	struct threadArgs *p;
	p = (struct threadArgs *)arg;

	int sockfd = p->sockfd, newfd = p->newfd;
	char buffer[BUFFER_SIZE];

	int wrongAttempts = 0;


	init_chat:

	recv(newfd,buffer,sizeof(buffer),0); //username
	int username = getnumber(buffer);
	recv(newfd,buffer,sizeof(buffer),0); //password
	int password = getnumber(buffer);



	sem_wait(&ACCESS);
	if(userID[username] == -1){ //new user
		userID[username] = 1;
		userPWD[username] = password;
		loggedIN[username] = newfd;
		strcpy(buffer,"Registered & Logged IN....");
	}
	else{
		if(userPWD[username] != password){
			wrongAttempts += 1;
			if(wrongAttempts == 3){
				strcpy(buffer,"Max. number of attempts exceeded....");
				sem_post(&ACCESS);
				goto end_chat;
			}
			else{
				strcpy(buffer,"Wrong Password, Try again...."); 
				send(newfd,buffer,(size_t)sizeof(buffer),0); //sending status back to user
				sem_post(&ACCESS);
				goto init_chat;
			}
		}
		else{
			loggedIN[username] = newfd;
			strcpy(buffer,"Logged IN....");
		}
	}
	sem_post(&ACCESS);
	send(newfd,buffer,(size_t)sizeof(buffer),0);
	sem_wait(&ACCESS);

	strcpy(buffer,"\n\nOnline User(s):");
	send(newfd,buffer,(size_t)sizeof(buffer),0);


	for(int i = 0; i < BUFFER_SIZE; i++){
		if(i != username && userID[i] == 1 && loggedIN[i] != -1){
			gcvt((double)i,10,buffer);
			send(newfd,buffer,(size_t)sizeof(buffer),0);
		}
	}
	sem_post(&ACCESS);
	strcpy(buffer,"END");
	send(newfd,buffer,(size_t)sizeof(buffer),0);


	int recipentID,senderID;

	while(1){
		recv(newfd,buffer,sizeof(buffer),0); 
		if(strcasecmp(buffer,"logout") == 0){
			break;
		}
		recipentID = getnumber(buffer);
		recv(newfd,buffer,sizeof(buffer),0); 
		send(loggedIN[recipentID],buffer,(size_t)sizeof(buffer),0);
	}
	loggedIN[username] = -1;
	end_chat:
	close(newfd);
	pthread_exit(NULL);
}

int main(int argc,char *argv[]){

	if(argc != 2){
		printf("Usage: ./server <port_number>\n");
		exit(1);
	}

	signal(SIGINT,sig_handler);

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
	printf("Listening...\n");
	peer_addr_size = sizeof(struct sockaddr_storage);


	int s;
	pthread_t tid;

	memset(userID,-1,sizeof(userID));
	memset(loggedIN,-1,sizeof(loggedIN));

	sem_init(&ACCESS,0,1);
	while(1){
		newfd = accept(sockfd,(struct sockaddr *)&peer_addr,&peer_addr_size);
		arg = (struct threadArgs*)malloc(sizeof(struct threadArgs));
		arg->sockfd = sockfd, arg->newfd = newfd;
		s = pthread_create(&tid,NULL,threadFunction,(void *)arg);
	}
	pthread_exit(NULL);
	sem_destroy(&ACCESS);
	return 0;
}
