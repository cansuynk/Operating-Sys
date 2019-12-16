/* @Author
 * Student Name: Cansu YANIK
 * Student ID: 150170704
 * Date: 22.05.2019 */


/*to be able to involve an integer variable in strcat*/
#define _GNU_SOURCE
/*for shared memory and semaphores*/
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
/*to use fork*/
#include <unistd.h>
/*other necessary libraries*/
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>


/*Decrement operation for semaphore*/
void sem_wait_P(int sem_id, int val){ //P function
    struct sembuf semaphore;
    semaphore.sem_num = 0;
    semaphore.sem_op = (-1 * val);
    semaphore.sem_flg = 1;		/*relative: add sem op to value*/
    if(semop(sem_id, &semaphore, 1) < 0){
        printf("semop error...\n");
        printf("errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }
}

/*Increment operation for semaphore*/
void sem_signal_V(int sem_id, int val){ //V function
    struct sembuf semaphore;
    semaphore.sem_num = 0;
    semaphore.sem_op = val;
    semaphore.sem_flg = 1;		/*relative: add sem op to value*/
    if(semop(sem_id, &semaphore, 1) < 0){
        printf("semop error...\n");
        printf("errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }
}


int main(int argc, char *argv[]){
	
	/*The ftok function returns a key based on path and id that is usable in subsequent calls to semget() and shmget()*/
	
	const key_t KEYSEM = ftok(argv[0], 1);          /*Creation of key values for sem_sync.*/
    const key_t KEYSEM2 = ftok(argv[0], 2);          
    
    const key_t KEYSHM = ftok(argv[0], 3);			/*Creation of key value for shm_sync.*/
    const key_t KEYSHM2 = ftok(argv[0], 4);
    const key_t KEYSHM3 = ftok(argv[0], 5);
    const key_t KEYSHM4 = ftok(argv[0], 6);
    const key_t KEYSHM5 = ftok(argv[0], 7);
    
    int shm_id = 0;				/*shared memory id for the box color array*/
    int shm_id2 = 0;			/*shared memory id for the number of color change*/
    int shm_id3 = 0;			/*shared memory id for the index of current painting box*/
    int shm_id4 = 0;			/*shared memory id for an array to hold actual painting order*/
    int shm_id5 = 0;			/*shared memory id for the box number*/
    
    char *mem_ptr = NULL;   	/*shared memory area for the box color array*/
    int *mem_ptr2 = NULL;		/*shared memory area for the number of color change*/
    int *mem_ptr3 = NULL;		/*shared memory area for the index of current painting box*/
    int *mem_ptr4 = NULL;		/*shared memory area for an array to hold actual painting order*/
    int *mem_ptr5 = NULL;		/*shared memory area for the box number*/
	
	char *input_file_name = argv[1];	/*input and output file names are taken*/
	char *output_file_name = argv[2];
	
	FILE * input_file; 					/*input and output files*/
	FILE * output_file;
	
	int box_number = 0;				/*Holdstotal number of boxes*/
	char color;						/*Holds the one color letter*/
    int for_child_process = 0;		/*Semaphore id to create a synchronization between parent and box child processes*/
    int lock = 1;					/*Counting semaphore to create a locking mechanism between box processes(only two processes can enter CS at a time)*/
	
	int f;									/*Return value of fork()*/
	
	
	input_file = fopen(input_file_name, "r");	/*Opens input file and read number of boxes*/
	fscanf(input_file,"%d",&box_number);
	
	pid_t box_processes_id[box_number];		/*box processes ids*/
	
	int i, myIndex;						/*Order of running bos processes*/
	for ( i=0; i<box_number; i++ ){		/*Creates child processes*/
		
		f = fork();
		if(f==-1){
			printf("FORK error ... \n");
			exit(1);
		}
		if (f==0)
			break;
		box_processes_id[i] = f;	
		
	}
	
	/*Parent process*/
	if (f!=0){
		
		/*Creates a semaphore for mutual exclusion(value = 1) between box processes*/
		lock = semget(KEYSEM, 1, 0700|IPC_CREAT);
		semctl(lock, 0, SETVAL, 1);
		
		/*Creates a semaphore for semaphore (value = 0) between parent and box processes*/
		for_child_process = semget(KEYSEM2, 1, 0700|IPC_CREAT);
		semctl(for_child_process, 0, SETVAL, 0);
		
		/*Creates and attaches a shared memory area for box colors*/
		shm_id = shmget(KEYSHM, box_number * sizeof(char), IPC_CREAT|0700|S_IRUSR |S_IWUSR);
		mem_ptr = (char *) shmat(shm_id, 0, 0);
		
		/*Creates and attaches a shared memory area for color change value*/
		shm_id2 = shmget(KEYSHM2, 1 * sizeof(int), IPC_CREAT|0700|S_IRUSR |S_IWUSR);  
		mem_ptr2 = (int *) shmat(shm_id2, 0, 0);
		
		/*Creates and attaches a shared memory area for the index of current painting box*/
		shm_id3 = shmget(KEYSHM3, 1 * sizeof(int), IPC_CREAT|0700|S_IRUSR |S_IWUSR);   //current index
		mem_ptr3 = (int *) shmat(shm_id3, 0, 0);
		
		/*Creates and attaches a shared memory area for an array to hold actual painting order of boxes*/
		/*Based on these orders, output file will be filled with the box processes' ids and colors*/
		/*When a box process is working, it saves its painting order into its index*/
		/*Also, I want extra one integer space to be able to use it as a counter(for indexing)*/
		shm_id4 = shmget(KEYSHM4, (box_number+1) * sizeof(int), IPC_CREAT|0700|S_IRUSR |S_IWUSR); 
		mem_ptr4 = (int *) shmat(shm_id4, 0, 0);
		
		/*Creates and attaches a shared memory area for total number of boxes*/
		shm_id5 = shmget(KEYSHM5, 1* sizeof(int), IPC_CREAT|0700|S_IRUSR |S_IWUSR);  
		mem_ptr5 = (int *) shmat(shm_id5, 0, 0);
		
		int index = 0;
		//fscanf(input_file,"%d",&index);
		//index = 0;
		while(index < box_number){			/*Reads colors from file and saves them into an array*/
			fscanf(input_file,"%c",&color);
			if(color!='\n' && color!=' '){
				*(mem_ptr + index) = color;		/*Colors is saved into box color array*/
				*(mem_ptr4 + index) = 0;		/*Also, in this index, initial painting order which is zero(not known yet) is saved.*/
				index++;
			}
		}
		
		*mem_ptr2 = 1;					/*Since painting processes will start, the number of color change is 1 initially*/
		*mem_ptr3 = 0;					/*The current index of box which will be painted is initially 0(We start with the first box for painting process)*/
		*mem_ptr5 = box_number;			/*The total number of boxes is saved*/
		*(mem_ptr4 + box_number) = 1;	/*The last element is for indexing the processes, initially it is 1(The order of first process will be 1) */
		
			
		fclose(input_file);		/*Closes the input file*/
		
		sem_wait_P(for_child_process, box_number);   /*Waits until all boxes will be painted.*/
		
		
		/*After finishing box processes, we can fill the output file by using the actual painting order array which mem_ptr4 points*/
		output_file = fopen(output_file_name, "w");
		fprintf(output_file, "%d\n", *mem_ptr2);	/*Write the number of color change which mem_ptr2 points*/
		
		int painting_order = 1;					/*Now we can write the box process ids and thier colors to the file*/
		while(painting_order<=box_number){
			for(int m=0; m<box_number; m++){			/*For the current order, it search the corresponding box(index) which has this order number in the painting order array*/
				if(*(mem_ptr4 + m) == painting_order){	/*Example: if second box will be written, the box which has 2 in the array is searched and based on its index, its pid and color will be written*/
					fprintf(output_file, "%d  %c\n", box_processes_id[m], *(mem_ptr+m));
				}				
			}
			painting_order = painting_order + 1;	/*Next order*/
		}
		
		printf("All boxes are painted and the process is written to the output file.\n");
		fclose(output_file); 	/*Closes output file*/
		
		shmdt(mem_ptr);			/*Detaches the shared memory segments*/
		shmdt(mem_ptr2);
		shmdt(mem_ptr3);
		shmdt(mem_ptr4);
		shmdt(mem_ptr5);
		
		sleep(1);
		
		/*Removes the created semaphores and shared memories*/
		if(semctl(lock, 0, IPC_RMID) < 0)		printf("semctl error...\n");
		if(semctl(for_child_process, 0, IPC_RMID) < 0)		printf("semctl error...\n");
		if(shmctl(shm_id, IPC_RMID, 0) < 0)			printf("shmctl error...\n");
		if(shmctl(shm_id2, IPC_RMID, 0) < 0)			printf("shmctl error...\n");
		if(shmctl(shm_id3, IPC_RMID, 0) < 0)			printf("shmctl error...\n");
		if(shmctl(shm_id4, IPC_RMID, 0) < 0)			printf("shmctl error...\n");
		if(shmctl(shm_id5, IPC_RMID, 0) < 0)			printf("shmctl error...\n");
	
	}
	else{	/*Box processes*/
		myIndex = i;	/*Saves the index of process*/
		
		/*Returns the semaphore ids for KEYSEM and KEYSEM2*/
		lock = semget(KEYSEM, 1, 0);		
		for_child_process = semget(KEYSEM2, 1, 0);

		/*Returns the shared memory ids associated with the given keys and attaches the shared memory segments*/
		shm_id = shmget(KEYSHM, sizeof(char), 0);	/*shared memory area for box colors*/
		mem_ptr = (char *) shmat(shm_id, 0, 0);
		
		shm_id2 = shmget(KEYSHM2, sizeof(int), 0);	/*shared memory area for color change value*/
		mem_ptr2 = (int *) shmat(shm_id2, 0, 0);
		
		shm_id3 = shmget(KEYSHM3, sizeof(int), 0);	/*shared memory area for the index of current painting box*/
		mem_ptr3 = (int *) shmat(shm_id3, 0, 0);
		
		shm_id4 = shmget(KEYSHM4, sizeof(int), 0); 	/*shared memory area for an array to hold actual painting order of boxes*/
		mem_ptr4 = (int *) shmat(shm_id4, 0, 0);
		
		shm_id5 = shmget(KEYSHM5, sizeof(int), 0);	/*shared memory area for total number of boxes*/
		mem_ptr5= (int *) shmat(shm_id5, 0, 0);
		
		

		/*Processes which their colors are not same with the current process color, waits*/
		while(*(mem_ptr + myIndex) != *(mem_ptr + *mem_ptr3));		
		
		/*Processes which their index do not equal to current index, waits*/
		while(*mem_ptr3 != myIndex);	
				 
		/*mem_ptr4 points to actual painting order array*/
		/*The last element of this array holds the current number of order*/
		*(mem_ptr4 + myIndex) = *(mem_ptr4 + *mem_ptr5);	/*By using last element of this array, the order of the process is assigned here.*/	
		*(mem_ptr4 + *mem_ptr5) = *(mem_ptr4 + *mem_ptr5) + 1;  /*For the next process, the order value(last element of this array) increments*/
		*mem_ptr3 = *mem_ptr3 + 1;								/*We can change the current box process array index*/
		
		/*Until reaching the end of the color array, we search another process which has the same color with the current process*/
		/*If there is no box which has same color left to be painted, we can pass the next new color to paint*/
		while((*(mem_ptr + myIndex) != *(mem_ptr + *mem_ptr3) ) && *mem_ptr3 != *mem_ptr5){
			*mem_ptr3 = *mem_ptr3 + 1;
		}
		if(*mem_ptr3 == *mem_ptr5){	
			
			for(int k=0; k< *mem_ptr5 ; k++){
				if(*(mem_ptr4 + k) == 0){
					*mem_ptr3 = k;
					*mem_ptr2 = *mem_ptr2 + 1;
					break;
				}
				
			}
		}
		/*Other process will wait the semaphore to enter CS*/
		sem_wait_P(lock, 1);	/*After box selection process is done, we can paint this box now.*/
		/*If the color is red, yellow or purple, process will take 1 sec., otherwise it will take 2 sec.*/
		if(*(mem_ptr+myIndex) == 'R' || *(mem_ptr+myIndex) == 'Y' || *(mem_ptr+myIndex) == 'P')	
			sleep(1);
		else 
			sleep(2);
			
		printf("%d  %c\n", getpid(), *(mem_ptr+myIndex));	/*To be able to follow the process, I wanted to print it.*/
		sem_signal_V(lock, 1);
		/*Makes the CS available again*/
		
		/*Detaches the shared memory segments*/
		shmdt(mem_ptr);
		shmdt(mem_ptr2);
		shmdt(mem_ptr3);
		shmdt(mem_ptr4);
		shmdt(mem_ptr5);
		
		sem_signal_V(for_child_process, 1); /*Increases the semaphore by 1 (synchronization with the parent)*/
		
	}
	
	return(0);
}
