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
#include<bits/stdc++.h>
using namespace std;

#define BUFFER_SIZE 1024
#define BACKLOG 10


set<string> username;
map<string,string> port;
map<string,int> bytesExchanged;

void sig_handler(int signo){
	if(signo == SIGINT){
		printf("\nServer : Exiting"), exit(0);
	}
}

struct threadArgs{
	int sockfd, newfd;
    char host[NI_MAXHOST], service[NI_MAXSERV];
};

struct threadArgs *ARG;

string getCppString(char *c){
	string a = "";
	for(int i = 0; c[i] != '\0'; i++) a += c[i];
	return a;
}

void copyIntoBuffer(char *a, string b){
	int n = b.length();
	for(int i = 0; i < n; i++) a[i] = b[i];
	a[n] = '\0';
}

void numberToChar(int n,char *a){
	vector<int> c;
	if(n == 0) c.push_back(0);
	else while(n!=0){c.push_back(n%10); n/=10;}
	n = c.size();
	for(int i = n-1, j = 0; i >= 0; i--, j++){
		a[j] = (char)(c[i] + '0');
	}
}

void *threadFunction(void *ARG){


	// printf("New Thread Created....\n");

	struct threadArgs *p;
	p = (struct threadArgs *)ARG;

	int sockfd = p->sockfd, newfd = p->newfd;
	char buffer[BUFFER_SIZE];
	
	strcpy(buffer,"Enter username : ");
	send(newfd,buffer,(size_t)sizeof(buffer),0);	

	memset(buffer,'\0',sizeof(buffer));
	recv(newfd,buffer,sizeof(buffer),0); //recv username

	string id = getCppString(buffer);
	int flag = 0;

	if(username.find(id) != username.end()){
		//send username already in use mesage
		strcpy(buffer,"Username already in use, try again");
		send(newfd,buffer,(size_t)sizeof(buffer),0);
		flag = 1;
		goto close_connection;
	}
	else{
		//You are successfully registered
		username.insert(id);
		port[id] = p->service;
		bytesExchanged[id] = 0;
		// set<string> username;
		// map<string,string> port;
		// map<string,int> bytesExchanged;

		strcpy(buffer,"Successfully registered");
		send(newfd,buffer,(size_t)sizeof(buffer),0);
		bytesExchanged[id] += strlen(buffer);
	}
	//////////////////////////////////////////////////
	//normal proceedings


	while(1){
		strcpy(buffer,"Enter command, (type 'exit' for terminating connection) : ");
		send(newfd,buffer,(size_t)sizeof(buffer),0);
		bytesExchanged[id] += strlen(buffer);


		recv(newfd,buffer,sizeof(buffer),0); //this is the command
		bytesExchanged[id] += strlen(buffer);
		if(strcasecmp(buffer,"who") == 0){


			strcpy(buffer,"loop");
			send(newfd,buffer,(size_t)sizeof(buffer),0);
			bytesExchanged[id] += strlen(buffer);

			for(set<string>::iterator i = username.begin(); i != username.end(); i++){
				// strcpy(buffer,"chai");
				copyIntoBuffer(buffer,*i);
				send(newfd,buffer,(size_t)sizeof(buffer),0);
				bytesExchanged[id] += strlen(buffer);


				copyIntoBuffer(buffer,port[*i]);
				send(newfd,buffer,(size_t)sizeof(buffer),0);					
				bytesExchanged[id] += strlen(buffer);

				memset(buffer,'\0',sizeof(buffer));
				numberToChar(bytesExchanged[*i],buffer);
				send(newfd,buffer,(size_t)sizeof(buffer),0);					
				bytesExchanged[id] += strlen(buffer);
			}
			strcpy(buffer,"endloop");
			send(newfd,buffer,(size_t)sizeof(buffer),0);
			bytesExchanged[id] += strlen(buffer);

		}
		else if(strcasecmp(buffer,"whoami") == 0){

			strcpy(buffer,"~");
			send(newfd,buffer,(size_t)sizeof(buffer),0);
			bytesExchanged[id] += strlen(buffer);


			copyIntoBuffer(buffer,id);
			send(newfd,buffer,(size_t)sizeof(buffer),0);
			bytesExchanged[id] += strlen(buffer);


			copyIntoBuffer(buffer,port[id]);
			send(newfd,buffer,(size_t)sizeof(buffer),0);
			bytesExchanged[id] += strlen(buffer);

			memset(buffer,'\0',sizeof(buffer));
			numberToChar(bytesExchanged[id],buffer);
			bytesExchanged[id] += strlen(buffer);
			send(newfd,buffer,(size_t)sizeof(buffer),0);					
			//one line
		}
		else if(strcasecmp(buffer,"exit") == 0){

			strcpy(buffer,"^");
			send(newfd,buffer,(size_t)sizeof(buffer),0);
			bytesExchanged[id] += strlen(buffer);

			goto close_connection;
			//one line
		}
		else{
			strcpy(buffer,"Unknown command, try again");
			send(newfd,buffer,(size_t)sizeof(buffer),0);	
			bytesExchanged[id] += strlen(buffer);

		}
	}
	close_connection:
	if(!flag)
		username.erase(id);
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
	printf("Listening...\n\n");
	peer_addr_size = sizeof(struct sockaddr_storage);


	int s;
	pthread_t tid;

	while(1){
		newfd = accept(sockfd,(struct sockaddr *)&peer_addr,&peer_addr_size);
		ARG = (struct threadArgs*)malloc(sizeof(struct threadArgs));
		int S = getnameinfo((struct sockaddr *) &peer_addr,
		                 peer_addr_size, ARG->host, NI_MAXHOST,
		                 ARG->service, NI_MAXSERV, NI_NUMERICSERV);
		ARG->sockfd = sockfd, ARG->newfd = newfd;
		s = pthread_create(&tid,NULL,threadFunction,(void *)ARG);
	}
	pthread_exit(NULL);
	return 0;
}
