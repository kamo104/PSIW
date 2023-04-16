#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>


int main(){
    int id, pid, i;
    volatile int *ptr;

    id = shmget(0x64, 2*sizeof(int), IPC_CREAT|0640);
    ptr = shmat(id,NULL,0);

    ptr[1] = 0;

    pid = fork();

    for(i=1;i<=100;i++){
        if(pid==0){
            while(ptr[1]==1);
            *ptr =i;
            ptr[1]=1;
        } 
        else{
            while(ptr[1]==0);
            printf("%d\n", *ptr);
            ptr[1]=0;
        } 
    }
    return 0;
}