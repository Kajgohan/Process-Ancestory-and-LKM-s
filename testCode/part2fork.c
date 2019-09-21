#include <sys/syscall.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>

// These values must match the syscall table
#define __NR_cs3013_syscall2 378
#define MAX 100


struct ancestry{
	pid_t ancestors[10];
	pid_t siblings[100];
	pid_t children[100];
};



long testCall2(unsigned short *target, struct ancestry *response){
	pid_t newProc = fork();
	if (newProc == 0){
		unsigned int pid = (unsigned int) getppid();
		*target = pid;
		long results = (long) syscall(__NR_cs3013_syscall2, target, response);
		return results;
	}
	else if (newProc > 0){
		wait(NULL);
	}
	return 0;
}


int main () {
	int i = 0;
	int k = 0;
	int e = 0;
	struct ancestry *response;
	response = malloc(sizeof(struct ancestry));
	unsigned short *target;
	target = malloc(sizeof(target));
	for(int w = 0; w < 100; w++){
		response->siblings[w] = -1;
		//printf("%d\n",response->siblings[w]);
	}
	for(int q = 0; q < 10; q++){
		response->ancestors[q] = -1;
	}
	for(int r = 0; r < 100; r++){
		response->children[r] = -1;
	}
  //printf("The return values of the system calls are:\n");
  printf("\tcs3013_syscall2: %ld\n",testCall2(target, response));
	//printf("The return value of the system call is: %ld\n",testCall2());
	for (i = 0; i < MAX; i++){
		if(response->children[i] != -1){
			printf("The process with a pid of %d is a child of this process!\n",response->children[i]);
		}
	}
	
	printf("\n\n");
	for (k = 0; k < MAX; k++){
		if(response->siblings[k] != -1){
			printf("The process with a pid of %d is a sibling of this process!\n",response->siblings[k]);
		}
	}
	
	printf("\n\n");
	for (e = 0; e < MAX; e++){
		if(response->ancestors[e] != -1){
			printf("The process with a pid of %d is an ancestor of this process!\n",response->ancestors[e]);
			//e++;
		}
	}
  return 0;
}
