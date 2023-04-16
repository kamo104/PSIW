// wzajemne wykluczenie czytających i wzajemne wykluczenie czytających, 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define MAX 10

struct mem {
    int buf[MAX];
    int read;
    int write;
} volatile *memory;

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

    //branie semafora
    semid = semget(0x63,4,IPC_CREAT|0640); 
    // sem 0 dla jakiegolokwiek producenta, 
    // sem 1 dla jakiegokolwiek czytelnika,
    // sem 2 dla wzajemnego wykluczenia producentów,
    // sem 3 dla wzajemnego wykluczenia konsumentów

    if(semid==-1){
        perror("Tworzenie semafora");
        exit(1);
    }
    if(semctl(semid, 0, SETVAL, (int)MAX)==-1){
        perror("Ustawianie wartości początkowej semafora");
        exit(1);
    }
    if(semctl(semid, 1, SETVAL, (int)0)==-1){
        perror("Ustawianie wartości początkowej semafora");
        exit(1);
    }
    if(semctl(semid, 2, SETVAL, (int)1)==-1){
        perror("Ustawianie wartości początkowej semafora");
        exit(1);
    }
    if(semctl(semid, 3, SETVAL, (int)1)==-1){
        perror("Ustawianie wartości początkowej semafora");
        exit(1);
    }

    //branie pamięci
    id = shmget(0x64, sizeof(memory), IPC_CREAT|0640);
    memory = shmat(id,NULL,0);

    pid = fork();
    pid = fork();   

    for(i=0;i<100;i++){
        if(pid==0){
            opusc(semid,0);
            
            opusc(semid,2);

            (*memory).buf[(*memory).write] = i;

            (*memory).write = ((*memory).write+1) % MAX;

            podnies(semid,2);

            podnies(semid,1);
        }
        else{
            podnies(semid,0);

            opusc(semid,3);

            printf("%d\n", (*memory).buf[(*memory).read]);
            fflush(stdout);
            (*memory).read = ((*memory).read+1) % MAX;

            podnies(semid,3);

            opusc(semid,1);
        } 
    }
    return 0;
}