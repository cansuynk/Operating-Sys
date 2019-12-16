#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>

int *ptr;
void *q2f(void *t) {
	long tid;
	*ptr=*ptr+2;
	tid= (long) t;
	printf("Process id=%d, Parent Process id=%d, Thread no= %ld Adress=%p,Value=%d\n", getpid(), getppid(), tid, ptr, *ptr);
	pthread_exit(NULL);
}
int main(void) {
	pthread_t threads[2];
	int r, i;
	ptr = (int *) malloc (sizeof(int));
	*ptr=0;
	for(i=0; i<2;i++) {
		r = pthread_create(&threads[i], NULL, q2f, (void *)(intptr_t)i);
		if (r) {
			printf("Error.\n");
			exit(1);
		}
	}
	printf("Process id=%d, Parent Process id=%d --- %d threads created.\n",
	getpid(), getppid(), i);
	pthread_exit(NULL);
	return 0;
}
