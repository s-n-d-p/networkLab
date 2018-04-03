#include<bits/stdc++.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<math.h>
#include<signal.h>
#include<pthread.h>
#include<set>
#include<map>
#include<semaphore.h>

// #define un fstream("username.txt","rw")
// #define pwd fstream("password.txt","rw")

FILE *u,*p,*m;

#define BUFFER_SIZE 1024
#define BACKLOG 5


using namespace std;

sem_t ACCESS;

void sig_handler(int signo){
	if(signo == SIGINT){
		printf("\nSMTP : Exiting"), exit(0);
	}
}

struct threadArgs{
	int sockfd, newfd;
};

void init(char *a,int size){
	memset(a,'\0',size);
}

struct threadArgs *ARG;


set<string> userDB;
map<string,string> pwDB;
map<string,vector<string> > mail;
map<string,vector<string> > mailSender;

string getCppString(char *a){
	string b = "";
	int i = 0;
	while(a[i] != '\0'){
		b += a[i++];
	}
	return b;
}

bool registered(string username){
	return userDB.find(username) != userDB.end();
}

bool pwdMatch(string username,string password){
	return pwDB[username] == password;
}

void copyMail(string b,char *a){
	init(a,BUFFER_SIZE);
	for(int i = 0; i < b.length(); i++){
		a[i] = b[i];
	}
}

void *threadFunction(void *arg){

	struct threadArgs *p;
	p = (struct threadArgs *)arg;
	int sockfd = p->sockfd, newfd = p->newfd;


	//first find out what he wants to do
	//if sending message, then 1 msg per connection

	char buffer[BUFFER_SIZE];

	//reading username
	strcpy(buffer,"Username : ");
	send(newfd,buffer,(size_t)sizeof(buffer),0);
	init(buffer,BUFFER_SIZE);
	recv(newfd,buffer,sizeof(buffer),0);
	string username = getCppString(buffer);
	strcpy(buffer,"Password : ");
	send(newfd,buffer,(size_t)sizeof(buffer),0);	
	init(buffer,BUFFER_SIZE);
	recv(newfd,buffer,sizeof(buffer),0);
	string password = getCppString(buffer);

	//accessing database
	init(buffer,BUFFER_SIZE);
	sem_wait(&ACCESS);	
	int flag = 0;

	if(!registered(username)){
		strcpy(buffer,"You are registered");
		userDB.insert(username);
		pwDB[username] = password;
		send(newfd,buffer,(size_t)sizeof(buffer),0);	
	}	
	else if(!pwdMatch(username,password)){
		strcpy(buffer,"Wrong password, try again later");
		send(newfd,buffer,(size_t)sizeof(buffer),0);	
		flag = 1;
	}
	else{
		//successfully logged in
		strcpy(buffer,"Logged In");
		send(newfd,buffer,(size_t)sizeof(buffer),0);	
	}
	sem_post(&ACCESS);

	if(flag == 1){
		goto terminate;
	}

	else{

		while(flag != 1){
			strcpy(buffer,"1. Check Mail\n");
			send(newfd,buffer,(size_t)sizeof(buffer),0);	
			strcpy(buffer,"2. Send Mail\n");
			send(newfd,buffer,(size_t)sizeof(buffer),0);	
			strcpy(buffer,"3. Logout\n");
			send(newfd,buffer,(size_t)sizeof(buffer),0);

			init(buffer,BUFFER_SIZE); 
			recv(newfd,buffer,sizeof(buffer),0);

			if(buffer[0] == '1'){
				sem_wait(&ACCESS);
				for(int i = 0; i < mail[username].size(); i++){

					copyMail(mailSender[username][i],buffer);
					send(newfd,buffer,(size_t)sizeof(buffer),0);						
					copyMail(mail[username][i],buffer);
					send(newfd,buffer,(size_t)sizeof(buffer),0);					
				}
				sem_post(&ACCESS);
				strcpy(buffer,"EOM");
				send(newfd,buffer,(size_t)sizeof(buffer),0);					

			}
			else if(buffer[0] == '2'){
				
				
				strcpy(buffer,"To: ");
				send(newfd,buffer,(size_t)sizeof(buffer),0);	
				init(buffer,BUFFER_SIZE);
				recv(newfd,buffer,sizeof(buffer),0);
				string recipent = getCppString(buffer);

				sem_wait(&ACCESS);
				if(!registered(recipent)){
					strcpy(buffer,"Invalid Email");
					send(newfd,buffer,(size_t)sizeof(buffer),0);
				}
				else{
					strcpy(buffer,"Message body : ");
					send(newfd,buffer,(size_t)sizeof(buffer),0);
					recv(newfd,buffer,sizeof(buffer),0);
					string message = getCppString(buffer);
					mail[recipent].push_back(message);	
					mailSender[recipent].push_back(username);
				}
				sem_post(&ACCESS);	
			}
			else{
				//logout the user
				flag = 1;
			}
		}

	}

	

	terminate:
	close(newfd);
	pthread_exit(NULL);
}

int main(int argc,char *argv[]){

	if(argc != 2){
		printf("Usage: ./senderSMTP <port_number>\n");
		exit(1);
	}

	signal(SIGINT,sig_handler);

	sem_init(&ACCESS,0,1);




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
		fprintf(stderr, "senderSMTP, getaddrinfo: %s\n",gai_strerror(status));
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
		perror("senderSMTP, listen");
		exit(EXIT_FAILURE);
	}
	printf("Listening...\n");
	peer_addr_size = sizeof(struct sockaddr_storage);


	int s;
	pthread_t tid;


	while(1){
		newfd = accept(sockfd,(struct sockaddr *)&peer_addr,&peer_addr_size);
		ARG = (struct threadArgs*)malloc(sizeof(struct threadArgs));
		ARG->sockfd = sockfd, ARG->newfd = newfd;
		s = pthread_create(&tid,NULL,threadFunction,(void *)ARG);
	}
	pthread_exit(NULL);
	return 0;
}
