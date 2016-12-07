//date: 2016.12.7
//author: junqing.ma
//This file use pthread mutex to synchronize the resource between the process 
#include <stdio.h>
#include <pthread.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#define SHARED_FILE "myshared"

int main(int argc, char* argv[])
{
    
    pthread_mutex_t mutex;
    pthread_mutexattr_t mutexattr;
    int* x;
    char* ptr;
    int rt;
    int ret;

    pthread_mutexattr_init(&mutexattr);
    pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&mutex, &mutexattr);
    

    rt = fork();

    if(rt == 0) //child process
    {
    	int child_shmid;
    	child_shmid = shm_open(SHARED_FILE, O_RDWR | O_CREAT, S_IRWXU | S_IRGRP);
    	if(-1 == child_shmid)
    	{
    		perror("shm_open failed in child process!");
    		exit(-1);
    	}
    	ftruncate(child_shmid, sizeof(int));
    	ptr = (char*)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, child_shmid, 0);

    	x = (int*)ptr;
    	int i;
    	for(i = 0; i < 10; i++)
    	{
    		ret = pthread_mutex_lock(&mutex);
            if(ret == 0)
            {
                (*x)++;
                printf("child process++ && x = %d\n", *x);
            }
    		
    		pthread_mutex_unlock(&mutex);
    		sleep(1);

    	}
    }
    else
    {
    	int father_shmid;
    	father_shmid = shm_open(SHARED_FILE, O_RDWR | O_CREAT, S_IRWXU | S_IRGRP);
    	ftruncate(father_shmid, sizeof(int));
    	ptr = (char*)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, father_shmid, 0);

    	x = (int*)ptr;
    	int i;
    	for(i = 0; i < 10; i++)
    	{
    		pthread_mutex_lock(&mutex);
    		(*x) += 2;
    		printf("father process+2 && x = %d\n", *x);
    		pthread_mutex_unlock(&mutex);
    		sleep(1);

    	}

    }

    shm_unlink(SHARED_FILE);
    munmap(ptr, sizeof(int));

    return 0;
}