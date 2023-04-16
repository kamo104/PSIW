#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>


const char* kierunki[2] = {"S->N","N->S"};
pid_t pid;

#define PODROZ 2

// semafory
static struct sembuf buf;

volatile int semid;

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





// pamięć współdzielona
volatile int mem_id;

struct most{
    int auta; // ilość aut znajdujących się aktualnie na moście
    int kierunek_jazdy; // 0: S->N , 1: N->S
    int czekajacy[2]; // ilość aut czekających na przejazd S->N [0] oraz N->S [1]
} volatile *memory;





// kolejka komunikatów
volatile int msg_id;

struct msg_buf {
   long mtype; // 1: S->N , 2: N->S
   int mvalue;
} msg_elem;

// #define SN 0 // kierunek_jazdy S->N
// #define NS 1 // kierunek_jazdy N->S



void opusc_most(int kierunek_jazdy){
    opusc(semid,0);

    memory->auta-=1;

    if(memory->auta!=0){ // jeśli nie byłem ostatnim autem na moście
        podnies(semid,0);
        return;
    }

    // jeśli byłem ostatnim autem na moście
    
    if(memory->czekajacy[!kierunek_jazdy]>0){ // jeśli ktoś z przeciwnego kierunku czeka
        msg_elem.mtype = !kierunek_jazdy+1;
        memory->kierunek_jazdy = !kierunek_jazdy;
        // wyślij n komunikatów do czekających z przeciwnego kierunku
        // postaw ich auta na moście i odejmij z czekających
        for(int i=0;i<memory->czekajacy[!kierunek_jazdy];i++){
            // wysłanie komunikatu
            if (msgsnd(msg_id, &msg_elem, sizeof(msg_elem.mvalue), 0) == -1){
                perror("Wyslanie komunikatu do jazdy");
                exit(1);
            }
            printf("[%d] %s %s\n",pid,"Wysłano pozwolenie do jazdy w kierunku", kierunki[!kierunek_jazdy]);
            memory->auta+=1;
        }
        memory->czekajacy[!kierunek_jazdy] = 0;
    }
    
    
    podnies(semid,0);
}

void jedz(int kierunek_jazdy){
    printf("[%d] %s %s\n",pid,"Wyruszam w kierunku",kierunki[kierunek_jazdy]);
    sleep(PODROZ);
    printf("[%d] %s %s\n",pid,"Koniec podrozy",kierunki[kierunek_jazdy]);
}



void start(int kierunek_jazdy){
    opusc(semid,0);
    // printf("%d")
    if((memory->czekajacy[!kierunek_jazdy]==0 && memory->kierunek_jazdy == kierunek_jazdy) || memory->auta==0){ // jeśli nikt na przeciwko nie czeka to jedź

        memory->auta+=1; // wjedź na most
        memory->kierunek_jazdy = kierunek_jazdy;

        podnies(semid,0);

        jedz(kierunek_jazdy);

        // kiedy skończymy jechać zdejmijmy sie z mostu
        opusc_most(kierunek_jazdy);

        return;

    }

    // jeśli ktoś na przeciwko czeka to ustaw się do czekania po swojej stronie
    memory->czekajacy[kierunek_jazdy] += 1;


    podnies(semid,0);


    printf("[%d] %s %s\n",pid,"Czekam na pozwolenie do jazdy w kierunku",kierunki[kierunek_jazdy]);
    // czekaj na komunikat z twoim kierunkiem jazdy
    if (msgrcv(msg_id, &msg_elem, sizeof(msg_elem.mvalue), kierunek_jazdy+1, 0) == -1){
        perror("Odebranie pozwolenia do jazdy");
        exit(1);
    }
    printf("[%d] %s %s\n",pid,"Odebrano pozwolenie do jazdy w kierunku",kierunki[kierunek_jazdy]);

    jedz(kierunek_jazdy);

    opusc_most(kierunek_jazdy);
}

// kierunek jazdy do podania przez paramentr przy uruchamianiu programu
// 0: S->N , 1: N->S
int main(int argc,char *argv[]){
    int kierunek_jazdy = 0;
    pid = getpid();

    switch(argc){
        case(1):{
            printf("[%d] %s\n",pid,"Kierunek jazdy auta nie zostal podany, kontynuowanie programu z kierunkiem S->N");
            break;
        }
        case(2):{
            kierunek_jazdy = atoi(argv[1]);
            if( kierunek_jazdy>1  || 0>kierunek_jazdy){
                perror("Niepoprawny kierunek jazdy");
                exit(1);
            }
            break;
        }
        default:{
            perror("Nieokreslona liczba argumentow");
            exit(1);
        }
    }


    // tworzenie/branie semafora
    semid = semget(456345,1,IPC_CREAT|IPC_EXCL|0660);
    if(semid==-1){
        // pierwszy=0;
        semid = semget(456345,1,IPC_CREAT|0660);
        if(semid==-1){
            perror("Tworzenie semafora");
            exit(1);
        }
    }
    else if(semctl(semid, 0, SETVAL, (int)1)==-1){ // jeśli nie istnieje semafor to trzeba zinicjalizować na nim odpowiednie wartości
        perror("Ustawianie wartosci poczatkowej na semaforze");
        exit(1);
    }
    
    // tworzenie/branie pamięci
    int pierwszy = 1;
    mem_id = shmget(456346, sizeof(memory), IPC_CREAT|IPC_EXCL|0660);
    if(mem_id==-1){
        pierwszy = 0;
        mem_id = shmget(456346, sizeof(memory), IPC_CREAT|0660);
        if(mem_id==-1){
            perror("Tworzenie pamieci wspoldzielonej");
            exit(1);
        }
    }

    memory = shmat(mem_id,NULL,0);

    if(pierwszy==1){ // jeśli nie istnieje współdzielona pamięć to trzeba zinicjalizować na niej odpowiednie wartości)
        memory->auta=0;
        memory->czekajacy[0]=0;
        memory->czekajacy[1]=0;
        memory->kierunek_jazdy=0;
    }

    // tworzenie/branie kolejki
    pierwszy=1; // jeśli nie istnieje kolejka komunikatów to trzeba zinicjalizować na niej odpowiednie wartości
    msg_id = msgget(456347, IPC_CREAT|IPC_EXCL|0660);
    if (msg_id == -1){
        pierwszy=0;

        msg_id = msgget(456347, IPC_CREAT|0660);
        if (msg_id == -1){
            perror("Utworzenie kolejki komunikatow");
            exit(1);
        }
   }

    

    start(kierunek_jazdy);



    // shmdt(&memory, NULL);
    return 0;
}