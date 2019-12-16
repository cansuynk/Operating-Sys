#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>

#define COUNT 2
#define MAX 4

int main (void)

{
	int i, ctr; 
	pid_t flist[MAX], tmp;

	for (i=0; i<MAX; i++)
		flist[i]=0;
	
	for (ctr=0; ctr<COUNT; ctr++) {
		tmp=fork();
		if (tmp > 0)
			flist[ctr]=tmp;
		else 
			break;

	}
	printf("My pid = %d. My parent pid = %d. My value of ctr = %d\n",
	getpid(), getppid(), ctr);

	for (i=0; i<MAX; i++)
		printf("(pid=%d): flist[%d]=%d\n", getpid(), i, flist[i]);
	if (tmp>0) {
		wait(NULL);
		printf("(pid=%d): Finished ...\n", getpid());
	}
	return (0);
}
