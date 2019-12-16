#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main (void){
int i, k[3] = {-1,-1,-1};
k[0] = fork();
for (i = 1; i<= 2; i++){
k[i] = fork();
if (k[i] == 0) break;
}
printf("Kimlik no: %d Anne kimlik no: %d\n", getpid(), getppid());
for (i = 0; i <= 2; i++)
printf("Kimlik no: %d, k[%d]: %d \n", getpid(), i, k[i]);
return 0;
}
