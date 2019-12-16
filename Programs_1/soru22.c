#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>

int main (void)

{
	pid_t f;
	int *ptr = (int *) malloc (sizeof(int));

	*ptr=1;
	f=fork();  
	*ptr= *ptr+2;
	printf("Process id=%d, Adress=%p, Value=%d\n", getpid(), ptr, *ptr);

	if (f>0) {
		*ptr=*ptr+3;
		printf("Process id=%d, Adress=%p, Value=%d\n", getpid(), ptr, *ptr);
		free(ptr);
		wait(NULL);
	}
	else {
		*ptr=*ptr+2;
	printf("Process id=%d, Adress=%p, Value=%d\n", getpid(), ptr, *ptr);
	}
	return(0); 

}
