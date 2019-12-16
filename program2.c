#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <string.h>



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

void sem_multi_signal(int semid, int val, int nsems){
	struct sembuf semaphore[2];
	int i;
	for (i = 0; i<nsems; i++){
		semaphore[i].sem_num=i;
		semaphore[i].sem_op=val;
		semaphore[i].sem_flg=1;
	}
	semop(semid, semaphore, 2);
	
}

void sem_multi_wait(int semid, int val, int nsems){
	struct sembuf semaphore[2];
	int i;
	for (i = 0; i<nsems; i++){
		semaphore[i].sem_num=i;
		semaphore[i].sem_op=(-1*val);
		semaphore[i].sem_flg=1;
	}
	semop(semid, semaphore, 2);
	
}

int main(int argc, char *argv[]){
	
	const key_t KEYSEM = ftok(argv[0], 1);          // Creation of key value for sem_sync.
    const key_t KEYSEM2 = ftok(argv[0], 2);          // Creation of key value for sem_sync.
    const key_t KEYSEM3 = ftok(argv[0], 3);          // Creation of key value for sem_sync.
    const key_t KEYSHM = ftok(argv[0], 4);
    const key_t KEYSHM2 = ftok(argv[0], 5);
    const key_t KEYSHM3 = ftok(argv[0], 6);
    const key_t KEYSHM4 = ftok(argv[0], 7);
    const key_t KEYSHM5 = ftok(argv[0], 8);
    
    int shm_id = 0;				//color array
    int shm_id2 = 0;			//color change
    int shm_id3 = 0;
    int shm_id4 = 0;
    int shm_id5 = 0;
    
    char *mem_ptr = NULL;   	//color array
    int *mem_ptr2 = NULL;		//color change
    int *mem_ptr3 = NULL;
    int *mem_ptr4 = NULL;
    int *mem_ptr5 = NULL;
	
	char *input_file_name = argv[1];
	char *output_file_name = argv[2];
	
	FILE * input_file; 
	FILE * output_file;
	
	int box_number = 0;
	char color;
	int for_parent_process = 0;
    int for_child_process = 0;
    int wait_process = 0;
    int lock = 0;
	
	input_file = fopen(input_file_name, "r");
	
	fscanf(input_file,"%d",&box_number);
	printf("Bu kadar kutucuk: %d\n",box_number);
	
	pid_t box_processes_id[box_number];
	
	int f;
	int i, myIndex;
	for ( i=0; i<box_number; i++ ){
		
		f = fork();
		if(f==-1){
			printf("FORK error ... \n");
			exit(1);
		}
		if (f==0)
			break;
		box_processes_id[i] = f;	
		
	}
	
	
	if (f!=0){
		
		lock = semget(KEYSEM, 1, 0700|IPC_CREAT);
		semctl(lock, 0, SETVAL, 1);
		
		for_child_process = semget(KEYSEM2, 1, 0700|IPC_CREAT);
		semctl(for_child_process, 0, SETVAL, 0);
		
		wait_process = semget(KEYSEM3, 2, 0700|IPC_CREAT);
		semctl(wait_process, 0, SETVAL, 1);	
		semctl(wait_process, 1, SETVAL, 1);	
		
		printf("Heyy ben parent processteyim.\n");
		
		shm_id = shmget(KEYSHM, box_number * sizeof(char), IPC_CREAT|0700|S_IRUSR |S_IWUSR);
		mem_ptr = (char *) shmat(shm_id, 0, 0);
		
		shm_id2 = shmget(KEYSHM2, 1 * sizeof(int), IPC_CREAT|0700|S_IRUSR |S_IWUSR);   //color change 
		mem_ptr2 = (int *) shmat(shm_id2, 0, 0);
		
		shm_id3 = shmget(KEYSHM3, 1 * sizeof(int), IPC_CREAT|0700|S_IRUSR |S_IWUSR);   //current index
		mem_ptr3 = (int *) shmat(shm_id3, 0, 0);
		
		shm_id4 = shmget(KEYSHM4, (box_number+1) * sizeof(int), IPC_CREAT|0700|S_IRUSR |S_IWUSR);  //new_order
		mem_ptr4 = (int *) shmat(shm_id4, 0, 0);
		
		shm_id5 = shmget(KEYSHM5, 1* sizeof(int), IPC_CREAT|0700|S_IRUSR |S_IWUSR);  
		mem_ptr5 = (int *) shmat(shm_id5, 0, 0);
		
		int index = 0;
		fscanf(input_file,"%d",&index);
		index = 0;
		while(index < box_number){
			fscanf(input_file,"%c",&color);
			if(color!='\n' && color!=' '){
				*(mem_ptr + index) = color;
				*(mem_ptr4 + index) = 0;
				index++;
			}
		}
		
		*mem_ptr2 = 1;
		*mem_ptr3 = 0;
		*mem_ptr5 = box_number;
		*(mem_ptr4 + box_number) = 1;
		
		/*
		for(int i = 0; i < box_number; i++){
			printf("%c ", *(mem_ptr + i));
		}*/
		
		
		fclose(input_file);
		
		
		sem_wait_P(for_child_process, box_number);   //wait until child processes were done.
		//wait(NULL);
		printf("Childlar bitti devam parenta\n");
		printf("toplam: %d\n", *mem_ptr2);
		int x = 1;
		output_file = fopen(output_file_name, "w");
		fprintf(output_file, "%d\n", *mem_ptr2);
		
		while(x<=box_number){
			for(int m=0; m<box_number; m++){
				if(*(mem_ptr4 + m) == x){
					printf("Bu %d  %c\n", box_processes_id[m], *(mem_ptr+m));
					fprintf(output_file, "%d  %c\n", box_processes_id[m], *(mem_ptr+m));
				}
					
				//printf("Bu %d", *(mem_ptr4+m));
			}
			x = x + 1;
		}
		
		fclose(output_file);
		shmdt(mem_ptr);
		shmdt(mem_ptr2);
		shmdt(mem_ptr3);
		shmdt(mem_ptr4);
		sleep(1);
		if(semctl(lock, 0, IPC_RMID) < 0)		printf("semctl error...\n");
		if(semctl(for_child_process, 0, IPC_RMID) < 0)		printf("semctl error...\n");
		if(shmctl(shm_id, IPC_RMID, 0) < 0)			printf("shmctl error...\n");
		if(shmctl(shm_id2, IPC_RMID, 0) < 0)			printf("shmctl error...\n");
		if(shmctl(shm_id3, IPC_RMID, 0) < 0)			printf("shmctl error...\n");
		if(shmctl(shm_id4, IPC_RMID, 0) < 0)			printf("shmctl error...\n");
	
	}
	else{
		myIndex = i;
		
		lock = semget(KEYSEM, 1, 0);
		for_child_process = semget(KEYSEM2, 1, 0);
		wait_process = semget(KEYSEM3, 2, 0);
		
		shm_id = shmget(KEYSHM, sizeof(char), 0);
		mem_ptr = (char *) shmat(shm_id, 0, 0);
		
		shm_id2 = shmget(KEYSHM2, sizeof(int), 0);
		mem_ptr2 = (int *) shmat(shm_id2, 0, 0);
		
		shm_id3 = shmget(KEYSHM3, sizeof(int), 0);
		mem_ptr3 = (int *) shmat(shm_id3, 0, 0);
		
		shm_id4 = shmget(KEYSHM4, sizeof(int), 0); 	//bool
		mem_ptr4 = (int *) shmat(shm_id4, 0, 0);
		
		shm_id5 = shmget(KEYSHM5, sizeof(int), 0);	//bos_number
		mem_ptr5= (int *) shmat(shm_id5, 0, 0);
		
		printf("ne bu %d  %d\n", *mem_ptr3, myIndex);
		
		
		/*
		if(*(mem_ptr+myIndex) != *(mem_ptr + *mem_ptr3)){
			
			sem_wait_P(wait_process, 1);
			*mem_ptr2 = *mem_ptr2 + 1;
			*mem_ptr3 = myIndex;
		}*/
		//else{	
		
		while(*mem_ptr3 != myIndex);
		sem_multi_wait(wait_process,1,2);
		//sem_wait_P(lock, 1);	
		//printf("Heyy ben child processteyim.\n");
		//printf("%c",*(mem_ptr + myIndex));
		
		printf("%d  %c\n", getpid(), *(mem_ptr+myIndex));
		
		if(*(mem_ptr+myIndex) == 'R' || *(mem_ptr+myIndex) == 'Y' || *(mem_ptr+myIndex) == 'P')
			sleep(1);
		else 
			sleep(2);
					 
		*mem_ptr3 = *mem_ptr3 + 1;
		*(mem_ptr4 + myIndex) = *(mem_ptr4 + *mem_ptr5);		//indexi atıyor
		*(mem_ptr4 + *mem_ptr5) = *(mem_ptr4 + *mem_ptr5) + 1;  //indexi artırıyor
		
		//sona gelmeyene kadar ve current colora eşit lamayana kadar(eşit olanı arıyor.)
		while((*(mem_ptr + myIndex) != *(mem_ptr + *mem_ptr3) ) && *mem_ptr3 != *mem_ptr5){
			*mem_ptr3 = *mem_ptr3 + 1;
		}
		if(*mem_ptr3 == *mem_ptr5){	//sona eşitse yeni renk geliyor.
			
			for(int k=0; k< *mem_ptr5 ; k++){
				if(*(mem_ptr4 + k) == 0){
					*mem_ptr3 = k;
					*mem_ptr2 = *mem_ptr2 + 1;
					break;
				}
				
			}
		}
		//printf("ne bu %d  %d\n", *mem_ptr3, myIndex);
		//sem_signal_V(lock, 1);
		sem_multi_signal(wait_process, 1, 2);
		
		shmdt(mem_ptr);
		shmdt(mem_ptr2);
		shmdt(mem_ptr3);
		shmdt(mem_ptr4);
		
		sem_signal_V(for_child_process, 1);
			
			
			//wait(NULL);
		//}
		//sem_signal_V(wait_process, 1);
		
		
	}
	
	
	
	return(0);
}
