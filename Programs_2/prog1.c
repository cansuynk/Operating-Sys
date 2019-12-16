#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
int x;
int main (void)
{
	x=1;
	int *ptr = (int *) malloc(sizeof(int));
	*ptr=5;
  pid_t f;
	
  f=fork();  //forking a child process
  if (f==-1) //fork is not successfull/error
  {
    printf("Error \n");
    exit(1);
  } 
  else if (f==0) //child process
  {
    printf("   Child: My process ID: %d \n", getpid());
    sleep(1); //waiting for 1 second
    printf("   Child: My parent's process ID: %d \n", getppid());
    printf("   x: %d \n", ++x);
    printf("   pointer address: %p \n", ptr);
    printf("   pointer value: %d \n", *ptr);
    *ptr=6;
    printf("   pointer value: %d \n", *ptr);
    exit(0);
  }
  else //parent process
  {
    printf("Parent: My process ID: %d \n", getpid());
    printf("Parent: My child's process ID: %d \n", f);
    printf("Parent: My parent's process ID: %d \n", getppid());
    printf("x: %d \n", --x);
    printf("pointer address: %p \n", ptr);
    printf("pointer value: %d \n", *ptr);
    *ptr=4;
    printf("pointer value: %d \n", *ptr);
    wait(NULL);  //waiting until child process exited
    printf("Parent: Terminating...\n");
    exit(0);
  }
  return (0);
}

