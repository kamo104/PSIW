#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define MAX 10


static struct sembuf buf;

void podnies(int semid, int semnum){
   buf.sem_num = semnum;
   buf.sem_op = +1;
   buf.sem_flg = 0;
   if (semop(semid, &buf, 1) == -1){
      perror("Podnoszenie semafora");
      exit(1);
   }
}

void opusc(int semid, int semnum){
   buf.sem_num = semnum;
   buf.sem_op = -1;
   buf.sem_flg = 0;
   if (semop(semid, &buf, 1) == -1){
        perror("Opuszczenie semafora");
        exit(1);
   }
}

int main(){
    int id, pid, i;
    int semid;
    volatile int *ptr;

    //branie semafora
    semid = semget(0x63,2,IPC_CREAT|0640);
    if(semid==-1){
        perror("Tworzenie semafora");
        exit(1);
    }
    if(semctl(semid, 0, SETVAL, (int)MAX)==-1){
        perror("Zerowanie semafora");
        exit(1);
    }

    if(semctl(semid, 1, SETVAL, (int)0)==-1){
        perror("Zerowanie semafora");
        exit(1);
    }

    //branie pamięci
    id = shmget(0x64, (MAX)*sizeof(int), IPC_CREAT|0640);
    ptr = shmat(id,NULL,0);

    pid = fork();

    for(i=0;i<100;i++){
        if(pid==0){
            opusc(semid,0);

            ptr[i%MAX] = i;

            podnies(semid,1);
        }
        else{
            podnies(semid,0);

            printf("%d\n", ptr[i%MAX]);
            fflush(stdout);

            opusc(semid,1);
        } 
    }
    return 0;
}