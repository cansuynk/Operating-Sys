#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
/*To use fork*/
#include <unistd.h>
/*To use thread*/
#include <pthread.h>

/*for handling signals*/
#include <signal.h>
#include <errno.h>
#include <time.h>

/* for shared memory and semaphores*/
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>

#define _GNU_SOURCE

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

void mysignal(int signum){ 
	printf("\nReceived signal with num = %d.\n", signum); 
}

void mysigset(int num){
    struct sigaction mysigaction;
    mysigaction.sa_handler = (void *)mysignal;
    mysigaction.sa_flags = 0;
    sigaction(num, &mysigaction, NULL);
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
int cmp(const void *a, const void *b){
		return ( *(int *)a - *(int *)b );	
}


#define KEYSEM ftok(strcat(get_current_dir_name(),argv[0]), 1);          // Creation of key value for sem_sync.
#define KEYSEM2 ftok(strcat(get_current_dir_name(),argv[0]), 2);          // Creation of key value for sem_sync.
#define KEYSHM ftok(strcat(get_current_dir_name(),argv[0]), 3);          // Creation of key value for shared memory.

int main(int argc, char *argv[]){
	mysigset(12);
	
	int shmid = 0;  /*shared memory id*/
	
	int f1;  //return value of fork();
	int f2;
	pid_t children[3];
	int N;
	int sem_sync = 0;
	int termSem = 0, lock = 0; /*semaphore ids*/
	int *mem_ptr = NULL;
    int *temp_ptr = NULL;
	
	srand(time(0)); /*create random numbers*/
	
	printf("Enter the N value (it has to be an even number!): ");
    scanf("%d", &N);
    if(N % 2){
        printf("You entered an odd number, program is terminating...\n");
        exit(EXIT_FAILURE);
    }
    
    f1 = fork();
    f2 = fork();
    
    
    if ( f1 > 0 && f2 > 0){ //parent
		termSem = semget(KEYSEM2, 1, 0700|IPC_CREAT);
		semctl( termSem, 0, SETVAL, 0);
		lock = semget(KEYSEM, 1,  0700|IPC_CREAT);
		semctl( lock, 0, SETVAL, 0);
		
		shmid = open_segment(KEYSHM, N + 1);
		mem_ptr = assign_segment(shm_id);               // Pointer to shared memory address.

        temp_ptr = mem_ptr;
        
        printf("Randomly created array by parent process: ");
		for(*temp_ptr++ = N; temp_ptr < mem_ptr + N + 1; *temp_ptr++ = rand()%100);     // Enter the randomly created elements to the shared memory.
		printElements(mem_ptr + 1, N);

		temp_ptr = NULL;
		
		kill(children[0], 12);                          // Trigger to start child 1.
        sleep(1);
		sem_wait_P(sem_sync, 1);
		
		wait(NULL);                                     // Wait for finishing of all children.
        printf("\nSmallest: %d\tLargest: %d\n", mem_ptr[1], mem_ptr[N]);
        
		shmdt(mem_ptr);
		sleep(1);
	}
    else if ( f1 == 0 && f2 > 0){  //1.child
		pause();		/*wait until receiving signal*/
		printf("\nFirst child is working.\n");
		
		/*returning semaphore id for these keys*/
		lock = semget(KEYSEM, 1, 0);
		termSem = semget(KEYSEM2, 1, 0);
		
		/*returning shared memory id*/
		shmid = open_segment(KEYSHM, N + 1); 
		
		mem_ptr = assign_segment(shmid);
		
		sem_wait_P(lock,1);
		
        temp_ptr = mem_ptr + 1;
        
        printf("Array elements that are child 1 is processing: ");
        printElements(temp_ptr, N/2);
        
        printf("The starting address of the array: %p, Finishing address: %p\n", (void *)temp_ptr, (void *)(temp_ptr + N/2));
        qsort((void *)temp_ptr, N/2, sizeof(int), cmp);
        
        printf("Child 2 has been sorted: ");
        printElements(temp_ptr, N/2);
        
        shmdt(mem_ptr);
		sleep(1);

		sem_signal_V(termSem, 1);
		exit(0);
		
	}
    else if ( f1 > 0 && f2 == 0){  //2. child
		pause();		/*wait until receiving signal*/
		printf("\nSecond child is working.\n");
		
		lock = semget(KEYSEM, 1, 0);
		termSem = semget(KEYSEM2, 1, 0);
		
		/*returning shared memory id*/
		shmid = open_segment(KEYSHM, N + 1); 
		
		mem_ptr = assign_segment(shmid);
		
		sem_wait_P(lock,1);
            
		temp_ptr = mem_ptr + 1 + N/2;                       // Pointer to starting address of array's second half.

		printf("Array elements that are child 2 is processing: ");
		printElements(temp_ptr, N/2);                       // Printing the elements of second half of array.

		printf("The starting address of the array: %p, Finishing address: %p\n", (void *)temp_ptr, (void *)(temp_ptr + N/2));

		qsort((void *)temp_ptr, N/2, sizeof(int), cmp);     // To sort array's second half.

		printf("Child 2 has been sorted: ");
		printElements(temp_ptr, N/2);

		shmdt(mem_ptr);
		sleep(1);

		sem_signal_V(termSem, 1);
		exit(0);
		
	}
    else if ( f1 == 0 && f2 == 0){  //3.child
		pause();		/*wait until receiving signal*/
		printf("\nSecond child is working.\n");
		
		lock = semget(KEYSEM, 1, 0);
		termSem = semget(KEYSEM2, 1, 0);
		
		/*returning shared memory id*/
		shmid = open_segment(KEYSHM, N + 1); 
		
		mem_ptr = assign_segment(shmid);
		
		sem_wait_P(lock,1);
		
		
		
		qsort((void *)(mem_ptr + 1), N, sizeof(int), cmp);      // Sorting all elements of array.
		printf("\nChild 3 is merging array 1 and 2: ");
        printElements(mem_ptr + 1, N);    
		
		shmdt(mem_ptr);
		sleep(1);

		sem_signal_V(termSem, 1);
		exit(0);
		
	}
    


}


