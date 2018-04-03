#include<string.h>
#include<pthread.h>
#include <stdio.h>
#include<stdlib.h>

#define NUM_THREADS	5

// struct thread_data{
// 	int thread_id;
// 	char message[100];
// };

// struct thread_data thread_data_array[NUM_THREADS];

void *printMessage(void *ptr){

	// struct thread_data *t = (struct thread_data *)ptr;

	// printf("%d\n",t->thread_id);
	// printf("%s\n",t->message);

	int *t = (int *)ptr;
	printf("Thread : %d\n",*t);
	pthread_exit(NULL);	
}

int main(){

	pthread_t thread[NUM_THREADS];
	// td threadArg[NUM_THREADS];

	for(int t = 0; t < NUM_THREADS; t++){
		printf("Creating thread : %d\n",t);
		int *m = malloc(sizeof(int)); *m = t;
		int rc = pthread_create(&thread[t],NULL,printMessage,(void *)m);
		if(rc){
			printf("Error : %d\n",rc);
			exit(-1);
		}
	}
	pthread_exit(NULL);
}