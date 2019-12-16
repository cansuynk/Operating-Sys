#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#include <pthread.h>
#include <stdint.h>
#include <sys/time.h>

int cmp(const void *a, const void *b){	
	return ( *(int *)a - *(int *)b );	
}

void mysignal(int signum){ 
	printf("\nReceived signal with num = %d.\n", signum); 
}

void mysigset(int num){
    struct sigaction mysigaction;
    mysigaction.sa_handler = (void *)mysignal;
    mysigaction.sa_flags = 0;
    sigaction(num, &mysigaction, NULL);
}

void sem_wait_P(int sem_id, int val){ //P function
    struct sembuf semaphore;
    semaphore.sem_num = 0;
    semaphore.sem_op = (-1 * val);
    semaphore.sem_flg = 1;
    if(semop(sem_id, &semaphore, 1) < 0){
        printf("semop error...\n");
        printf("errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }
}

void sem_signal_V(int sem_id, int val){ //V function
    struct sembuf semaphore;
    semaphore.sem_num = 0;
    semaphore.sem_op = val;
    semaphore.sem_flg = 1;
    if(semop(sem_id, &semaphore, 1) < 0){
        printf("semop error...\n");
        printf("errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }
}

int open_segment(key_t key_value, int in_size){
    int temp_shm_id;
    if((temp_shm_id = shmget(key_value, (in_size)*sizeof(int), IPC_CREAT|0666)) < 0){
        printf("shmget error...\n");
        printf("errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    return temp_shm_id;
}

int *assign_segment(int in_shm_id){
    int *temp_mem_ptr;
    if((temp_mem_ptr = (int *)shmat(in_shm_id, 0, 0)) == (void *)-1){
        printf("shmat error...\n");
        printf("errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    return temp_mem_ptr;
}

void printElements(int *start, int count){
    int *temp_ptr = start;
    printf("{%d", *(temp_ptr++));
    for( ; temp_ptr < start + count; printf(", %d", *temp_ptr++));
    printf("}\n");
}

int N, K;
key_t KEYSEM=0;
key_t KEYSEM2=0;

int sem_sync;
int lock;

int *mem_ptr;
int *mem_ptr2;

void *my_function(void *threadid) {
	
	
	long int i=(intptr_t)threadid;
	printf("%ld\n",i);
    int *temp_ptr2 = mem_ptr2 + i;
	
	sem_wait_P(lock, 1);
	
	int ratio = (N / K);
	printf("1.%ld    2.%ld   \n",(i * ratio), (i+1) * ratio-1);
	
	printElements((mem_ptr+(i * ratio)),ratio);
	
	qsort((void *)(mem_ptr+(i * ratio)), ratio, sizeof(int), cmp);
	printElements((mem_ptr+(i * ratio)),ratio);
	*temp_ptr2 = *(mem_ptr+((i+1) * ratio-1));
	
	fflush(stdout);
	sem_signal_V(lock, 1);
	sleep(1);
	sem_signal_V(sem_sync, 1);
	pthread_exit(NULL);
}

void *my_function2(void *threadid) {
	
	sem_wait_P(lock, 1);

	printElements(mem_ptr2, K);
		
	qsort((void *)(mem_ptr2), K , sizeof(int), cmp);
	
	printElements(mem_ptr2, K );
	printf("Largest: %d\n", mem_ptr2[K -1]);

	fflush(stdout);
	sem_signal_V(lock, 1);
	sleep(1);
	sem_signal_V(sem_sync, 1);
	pthread_exit(NULL);
	
}

int main(int argc, char *argv[])
{
	mysigset(12);
    
    KEYSEM = ftok(argv[0], 1);          // Creation of key value for sem_sync.
    KEYSEM2 = ftok(argv[0], 2);          // Creation of key value for sem_sync.

    int shm_id = 0;
  
    int *temp_ptr = NULL;
    
    sem_sync = 0;
    lock = 0;
    
    char *parameter1 = argv[1];
	char *parameter2 = argv[2];
	
	N = atoi(parameter1);
    K = atoi(parameter2);
    
    srand(time(0));            
    
    mem_ptr = (int *)malloc(N*sizeof(int));   
    mem_ptr2 = (int *)malloc(K*sizeof(int));          
	
    
	pthread_t threads[K+1];
	
	sem_sync = semget(KEYSEM2, 1, 0700|IPC_CREAT);
	semctl(sem_sync, 0, SETVAL, 0);
	
	lock = semget(KEYSEM, 1, 0700|IPC_CREAT);
	semctl(lock, 0, SETVAL, 1);
	
	temp_ptr = mem_ptr;

	printf("Randomly created array by parent process: ");
	for(temp_ptr = mem_ptr; temp_ptr < mem_ptr + N; *temp_ptr++ = rand()%100);     // Enter the randomly created elements to the shared memory.
	printElements(mem_ptr, N);

	temp_ptr = NULL;
	
	int rc, i;

	for(i=0; i<K;i++) {
		rc = pthread_create(&threads[i], NULL,my_function, (void *)(intptr_t)i);
		if (rc) {
		  printf("thread creation error ...\n");
		  exit(-1);
		}
	}	
	sem_wait_P(sem_sync, K);
	
	rc = pthread_create(&threads[i], NULL, my_function2, (void *)(intptr_t)i);
	if (rc) {
	  printf("thread creation error ...\n");
	  exit(-1);
	}

	sem_wait_P(sem_sync, 1);
	
	sleep(1);
	if(semctl(sem_sync, 0, IPC_RMID) < 0)		printf("semctl error...\n");
	if(semctl(lock, 0, IPC_RMID) < 0)		printf("semctl error...\n");
	
	free(mem_ptr);
	free(mem_ptr2);

	return 0;
}
