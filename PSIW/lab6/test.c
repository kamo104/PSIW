#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>


int main(){

    int id = shmget(10000000,100,IPC_CREAT|0600);

    if(id==-1){
        perror("costam");
        exit(1);
    }

    int *p;
    p = shmat(id,NULL,0);
    assert(p!=NULL);
    printf("%d\n", *p);
    *p=7;

	exit(0);
}
