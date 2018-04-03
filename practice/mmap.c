#include<sys/mman.h>
#include<unistd.h>
#include<stdio.h>
#include<semaphore.h>
#include<fcntl.h>
#include <sys/stat.h>
#include<stdlib.h>

#define BUFFER_SIZE 500

int *p;

sem_t *sem;

void child(){

    // sem_t *sem = sem_open(SNAME,0);
    sem_wait(sem);
    for(int i = 0 ; i < BUFFER_SIZE; i++){
            *(p+i) = 1000;
    }
    sem_post(sem);
    exit(0);
    
}

int main(){
    pid_t pid;


    p = mmap(NULL,(size_t)BUFFER_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANON,-1,0);
    sem = mmap(NULL,(size_t)sizeof(sem_t *),PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANON,-1,0);

    sem_init(sem,1,1);

    for(int i = 0 ; i < BUFFER_SIZE; i++){
        *(p+i) = i+1;
    }

    if((pid = fork()) == 0){
        child();
    }
    else{
        sem_wait(sem);
        for(int i = 0 ; i < BUFFER_SIZE; i++){
            printf("%d ",*(p+i));
            // printf("Parent");
        }
        sem_post(sem);
        wait(pid);

    }


    munmap(sem,sizeof(sem_t *));
    munmap(p,BUFFER_SIZE);

}