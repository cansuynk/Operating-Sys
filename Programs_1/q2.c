#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>

int main (void) {
	pid_t f;
	int *ptr = (int *) malloc (sizeof(int));
	*ptr=1;
	printf("Process id=%d, Parent Process id=%d, Adress=%p, Value=%d\n", getpid(),
	getppid(), ptr, *ptr);
	f=fork();
	if (f==-1) {
		printf("Error.\n");
		exit(1);
	}
	else {
		*ptr=*ptr+2;
		printf("Process id=%d, Parent Process id=%d, Adress=%p, Value=%d\n",
		getpid(), getppid(), ptr, *ptr);
		if (f!=0) {
			wait(NULL);
			printf("Process %d is now exiting...\n", getpid());
		}
		exit(0);
	}
	return(0);
}
