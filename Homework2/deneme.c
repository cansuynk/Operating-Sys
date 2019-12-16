#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>




int main(int argc, char *argv[]){
	
	
	char *parameter1 = argv[1];
	char *parameter2 = argv[2];
	
	int N = atoi(parameter1);
    int K = atoi(parameter2);
    
      
   int child[K];
    
   int i, myOrder = 0;
    
   for ( i=0; i<K; i++ ){
		child[i] = i;
		printf("%d ", child[i]);
		
	}
	

	
	return 0;

}
