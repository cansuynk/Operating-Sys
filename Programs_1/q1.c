#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>

#define COUNT 2
int main (void) {
	int i, f;
	for (i=0; i<COUNT; i++) {
		f=fork();
		if (f == -1) {
			printf("Error \n");
			exit(1);
		}
		if(f==0)
			break;
	}
	if (f==0) {
		printf("CHILD: Process id=%d, Parent Process id=%d\n", getpid(), getppid());
		sleep(1);
	}
	else {
		printf("PARENT: Process id=%d, Parent Process id=%d\n", getpid(),
		getppid());
		wait(NULL);
	}
	return (0);
}
