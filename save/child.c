#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc,char * argv[])
{

	int shmid,
	    i,
	    *count,
	    tmp;

	key_t k = ftok("/tmp",1);
	assert(k!=-1);

	shmid=shmget(k, sizeof(int), 0);
	assert(shmid >= 0);

	count = (int*)shmat(shmid,NULL,0);

	tmp = (int)strtol(argv[1],NULL,10);
	for(int i = 0 ; i < 10000 ; i++){
		*count += tmp;
	}
	//assert(shmctl(shmid,IPC_RMID,0) >=0);
}