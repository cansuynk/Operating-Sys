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

int main(int argc, char *argv[]){

    mysigset(12);
    
    const key_t KEYSEM = ftok(argv[0], 1);          // Creation of key value for sem_sync.
    const key_t KEYSEM2 = ftok(argv[0], 2);          // Creation of key value for sem_sync.
    const key_t KEYSHM = ftok(argv[0], 3);
    const key_t KEYSHM2 = ftok(argv[0], 4);
    const key_t KEYSHM3 = ftok(argv[0], 5);

    int shm_id = 0;
    int shm_id2 = 0;
    int shm_id3 = 0;
    
    int *mem_ptr = NULL;
    int *mem_ptr2 = NULL;
    int *mem_ptr3 = NULL;
    int *temp_ptr = NULL;
    int *temp_ptr2 = NULL;
    int *temp_ptr3 = NULL;
    int *temp_ptr4 = NULL;
    
    int sem_sync = 0;
    int lock = 0;
    int f;
    
    char *parameter1 = argv[1];
	char *parameter2 = argv[2];
	
	int N = atoi(parameter1);
    int K = atoi(parameter2);
    
    pid_t child[K];

    srand(time(0));                                 // To create random numbers.

    
    int i, myOrder = 1;
    
    for ( i=0; i<K; i++ ){
		
		f = fork();
		if(f==-1){
			printf("FORK error ... \n");
			exit(1);
		}
		if (f==0)
			break;
		child[i] = f;	
		
	}
	
	if (f!=0){
		sem_sync = semget(KEYSEM2, 1, 0700|IPC_CREAT);
		semctl(sem_sync, 0, SETVAL, 0);
		
		lock = semget(KEYSEM, 1, 0700|IPC_CREAT);
		semctl(lock, 0, SETVAL, 1);
		
		
		//shm_id = shmget(key_value, (N+1)*sizeof(int), IPC_CREAT|0666))
		shm_id = open_segment(KEYSHM, N + 1);           // Creation of shared memory.
		//mem_ptr = (int *)shmat(in_shm_id, 0, 0))
		mem_ptr = assign_segment(shm_id);               // Pointer to shared memory address.

		temp_ptr = mem_ptr;

		printf("Randomly created array by parent process: ");
		for(temp_ptr = mem_ptr; temp_ptr < mem_ptr + N; *temp_ptr++ = rand()%100);     // Enter the randomly created elements to the shared memory.
		printElements(mem_ptr, N);

		temp_ptr = NULL;
		
		//shm_id2 = shmget(key2, sizeof(int, 0700|IPC_CREAT));
		shm_id2 = open_segment(KEYSHM2, 2);           	// Creation of shared memory.
		mem_ptr2 = assign_segment(shm_id2);               // Pointer to shared memory address.
		//memptr2 = (int *) shmat(shm_id2, 0, 0);
		temp_ptr2 = mem_ptr2;
		*temp_ptr2++ = N;
		*temp_ptr2 = K;
		
		printf("N: %d, K: %d \n", *mem_ptr2, *(mem_ptr2+1));
		
		temp_ptr2 = NULL;
		
		
		shm_id3 = open_segment(KEYSHM3, K);           	// Creation of shared memory.
		mem_ptr3 = assign_segment(shm_id3);               // Pointer to shared memory address.
		printf("The List for the largest numbers of the sublists is created.\n");
		
		sleep(K);
		
		printf("Parent has created resources. \n");
		printf("Now, it will start the child processes. \n");
		
		
		for(i=0; i<K; i++){
			kill(child[i],12);
		}
		
		sem_wait_P(sem_sync, K);
		printf("All child processes are finished \n");
		
		printElements(mem_ptr3, K);
		
		qsort((void *)(mem_ptr3), K, sizeof(int), cmp);
		
		printElements(mem_ptr3, K);
		printf("Largest: %d\n", mem_ptr3[K-1]);
		
		shmdt(mem_ptr);
		shmdt(mem_ptr2);
		shmdt(mem_ptr3);
		
		sleep(1);
		if(semctl(sem_sync, 0, IPC_RMID) < 0)		printf("semctl error...\n");
		if(semctl(lock, 0, IPC_RMID) < 0)		printf("semctl error...\n");
		if(shmctl(shm_id, IPC_RMID, 0) < 0)			printf("shmctl error...\n");
		//semctl(lock, 0, IPC_RMID, 0);
		exit(0);
		
		
	}
	else{
		myOrder = i;
		pause();
		
		printf("Child %d is starting ... \n", myOrder);
		
		lock = semget(KEYSEM, 1, 0);		//synchronization between child processes mutual exlusion 
		sem_sync = semget(KEYSEM2, 1, 0);
		
		
		shm_id = open_segment(KEYSHM, N);        
        mem_ptr = assign_segment(shm_id);
        temp_ptr = mem_ptr;
        
		shm_id2 = open_segment(KEYSHM2, 2);        
        mem_ptr2 = assign_segment(shm_id2);
        temp_ptr2 = mem_ptr2;   //N
        temp_ptr3 = mem_ptr2 + 1;   //K
        
		shm_id3 = open_segment(KEYSHM3,  K );        
        mem_ptr3 = assign_segment(shm_id3);
        temp_ptr4 = mem_ptr3 + i;
        
        sem_wait_P(lock, 1);
        int ratio = (*temp_ptr2 / *temp_ptr3);
        //printf("1. %d    2.%d   \n",(i * ratio), (i+1) * ratio-1);
        printf("Array elements that are child %d is processing: ", myOrder);
        printElements((temp_ptr+(i * ratio)),ratio);

        printf("The starting address of the array: %p, Finishing address: %p\n", (void *)(temp_ptr+(i * ratio)), (void *)(temp_ptr + (i+1) * ratio-1));

        qsort((void *)(temp_ptr+(i * ratio)), ratio, sizeof(int), cmp);

        printf("Child %d has been sorted: ", myOrder);
        printElements((temp_ptr+(i * ratio)),ratio);
        
        *temp_ptr4 = *(temp_ptr+((i+1) * ratio-1));
        printf("BUUU: %d ",  *temp_ptr4);
        
        sem_signal_V(lock, 1);
        sleep(1); 
        
		shmdt(mem_ptr);
		shmdt(mem_ptr2);
		shmdt(mem_ptr3);
               

        sem_signal_V(sem_sync, 1);
        
        exit(EXIT_SUCCESS);
		
	}
}    
