#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>

int tmp=0;

void *my_function(void *threadid) {
  tmp=tmp+1;
  printf("%ld: Value= %d\n", (intptr_t) threadid, tmp);
  pthread_exit(NULL);
}

int main(void)
{
  pthread_t threads[3];
  int rc, i;
   
  for(i=0; i<3;i++) {
    rc = pthread_create(&threads[i], NULL,
                  my_function, (void *)(intptr_t)i);
    if (rc) {
      printf("thread creation error ...\n");
      exit(-1);
    }
  }
  printf("main(): Created %d threads.\n", i);
  pthread_exit(NULL);
  printf("deneme");
  return 0;
}
