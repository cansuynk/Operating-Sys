/************************
 * Ahmed Yasin KUL      *
 * 070150136            *
*************************/

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

int create_semaphore(key_t key_value, int mode){ // mode is 0 for sync and 1 for mutual exclusion.
    int result;
    if((result = semget(key_value, 1, 0700|IPC_CREAT)) < 0){
        printf("semget error...\n");
        printf("errno: %d", errno);
        exit(EXIT_FAILURE);
    }

    union semun{
        int val;
        struct semid_ds *buf;
        ushort *array;
    } sem_val;

    sem_val.val = mode;
    if(semctl(result, 0, SETVAL, sem_val) < 0){
        printf("semctl error,,,\n");
        printf("errno: %d", errno);
        exit(EXIT_FAILURE);
    }
    return result;
}

void printElements(int *start, int count){
    int *temp_ptr = start;
    printf("{%d", *(temp_ptr++));
    for( ; temp_ptr < start + count; printf(", %d", *temp_ptr++));
    printf("}\n");
}

int sem_get(key_t key_value){
    int result;
    if((result = semget(key_value, 1, 0)) < 0){
        printf("sem_sync semget error...\n");
        printf("errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    return result;
}



int main(int argc, char *argv[]){

    mysigset(12);
    
    const key_t KEYSEM = ftok(argv[0], 1);          // Creation of key value for sem_sync.
    const key_t KEYSEM2 = ftok(argv[0], 2);          // Creation of key value for sem_sync.
    const key_t KEYSHM = ftok(argv[0], 3);

    int shm_id = 0;
    int *mem_ptr = NULL;
    int *temp_ptr = NULL;
    pid_t children[3];
    int sem_sync = 0;
    int lock = 0;
    int N;

    srand(time(0));                                 // To create random numbers.

    printf("Enter the N value (it has to be an even number!): ");
    scanf("%d", &N);
    if(N % 2){
        printf("You entered an odd number, program is terminating...\n");
        exit(EXIT_FAILURE);
    }
    //fflush(stdout);                                 // Cleaning stdout to prevent printf fork anomaly.
    children[0] = fork();
    
    if(children[0] > 0){
		
        children[1] = fork();
        
        if(children[1] > 0){
            children[2] = fork();
            
            if(children[2] > 0){
                //sem_sync = create_semaphore(KEYSEM, 0);        // Initialization of semaphore to sync between parent process and child 1 and 2.
				
				sem_sync = semget(KEYSEM2, 1, 0700|IPC_CREAT);
				semctl(sem_sync, 0, SETVAL, 0);
				
				lock = semget(KEYSEM, 1, 0700|IPC_CREAT);
				semctl(lock, 0, SETVAL, 1);
				
				
                shm_id = open_segment(KEYSHM, N + 1);           // Creation of shared memory.
                mem_ptr = assign_segment(shm_id);               // Pointer to shared memory address.

                temp_ptr = mem_ptr;

                printf("Randomly created array by parent process: ");
                for(*temp_ptr++ = N; temp_ptr < mem_ptr + N + 1; *temp_ptr++ = rand()%100);     // Enter the randomly created elements to the shared memory.
                printElements(mem_ptr + 1, N);

                temp_ptr = NULL;
                
                kill(children[0], 12);                          // Trigger to start child 1.
				sleep(1);
                sem_wait_P(sem_sync, 1);

                kill(children[1], 12);                          // Trigger to start child 2.
                sleep(1);
                sem_wait_P(sem_sync, 1);
    
                kill(children[2], 12);                          // Trigger to start child 3.
                sleep(1);
                sem_wait_P(sem_sync, 1);                        // Wait for completing of child 3's work.

                wait(NULL);                                     // Wait for finishing of all children.
                printf("\nSmallest: %d\tLargest: %d\n", mem_ptr[1], mem_ptr[N]);
                
                //detach_memory(mem_ptr);
                shmdt(mem_ptr);
                sleep(1);
                if(semctl(sem_sync, 0, IPC_RMID) < 0)		printf("semctl error...\n");
                if(shmctl(shm_id, IPC_RMID, 0) < 0)			printf("shmctl error...\n");
            }
            
            else{
                pause();
                printf("\nThird child is working.\n");

                //sem_sync = sem_get(KEYSEM);
                lock = semget(KEYSEM, 1, 0);
				sem_sync = semget(KEYSEM2, 1, 0);

                shm_id = open_segment(KEYSHM, N + 1);
                mem_ptr = assign_segment(shm_id);

                qsort((void *)(mem_ptr + 1), N, sizeof(int), cmp);      // Sorting all elements of array.

                printf("\nChild 3 is merging array 1 and 2: ");
                printElements(mem_ptr + 1, N);                          // Printing all elements of array.

                //detach_memory(mem_ptr);
                 shmdt(mem_ptr);
                sleep(1);

                sem_signal_V(sem_sync, 1);
                exit(0);
            }
        }
        
        else{
            pause();
            printf("\nSecond child is working.\n");

           //sem_sync = sem_get(KEYSEM);
           lock = semget(KEYSEM, 1, 0);
			sem_sync = semget(KEYSEM2, 1, 0);
           
            shm_id = open_segment(KEYSHM, N + 1);

            mem_ptr = assign_segment(shm_id);
            
            temp_ptr = mem_ptr + 1 + N/2;                       // Pointer to starting address of array's second half.

            printf("Array elements that are child 2 is processing: ");
            printElements(temp_ptr, N/2);                       // Printing the elements of second half of array.

            printf("The starting address of the array: %p, Finishing address: %p\n", (void *)temp_ptr, (void *)(temp_ptr + N/2));

            qsort((void *)temp_ptr, N/2, sizeof(int), cmp);     // To sort array's second half.

            printf("Child 2 has been sorted: ");
            printElements(temp_ptr, N/2);

            //detach_memory(mem_ptr);
			 shmdt(mem_ptr);
            sleep(1);

            sem_signal_V(sem_sync, 1);
            exit(0);
        }
    }
    
    else{
        pause();
        printf("\nFirst child is working.\n");

        //sem_sync = sem_get(KEYSEM);
        lock = semget(KEYSEM, 1, 0);
		sem_sync = semget(KEYSEM2, 1, 0);
        shm_id = open_segment(KEYSHM, N + 1);        

        mem_ptr = assign_segment(shm_id);

        temp_ptr = mem_ptr + 1;

        printf("Array elements that are child 1 is processing: ");
        printElements(temp_ptr, N/2);

        printf("The starting address of the array: %p, Finishing address: %p\n", (void *)temp_ptr, (void *)(temp_ptr + N/2));

        qsort((void *)temp_ptr, N/2, sizeof(int), cmp);

        printf("Child 1 has been sorted: ");
        printElements(temp_ptr, N/2);
        
        //detach_memory(mem_ptr);
		shmdt(mem_ptr);
        sleep(1);        

        sem_signal_V(sem_sync, 1);
        
        exit(EXIT_SUCCESS);
    }
    return 0;
}
