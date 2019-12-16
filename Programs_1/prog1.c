#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
int x=0;
int main (void)
{
  pid_t f;

  f=fork();
  if (f==-1)
  {
    printf("Error \n");
    exit(1);
  } 
  else if (f==0)
  {
    printf("   Child: My process ID: %d \n", getpid());
    sleep(1);
    printf("   Child: My parent's process ID: %d \n", getppid());
    printf("   x: %d \n", ++x);
    exit(0);
  }
  else
  {
    printf("Parent: My process ID: %d \n", getpid());
    printf("Parent: My parent's process ID: %d \n", getppid());
    printf("Parent: My child's process ID: %d \n", f);
    printf("x: %d \n", --x);
    wait(NULL);
    printf("x2: %d \n", x);
    printf("Parent: Terminating...\n");
    exit(0);
  }
  return (0);
}

