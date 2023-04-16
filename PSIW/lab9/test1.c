#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MAX 10

struct buf_elem {
   long mtype;
   int mvalue;
};
#define PUSTY 1
#define PELNY 2

int main(){
   int msgid, i;
   struct buf_elem elem;
   
   msgid = msgget(45281, IPC_CREAT|IPC_EXCL|0600);
   if (msgid == -1){
      msgid = msgget(45281, IPC_CREAT|0600);
      if (msgid == -1){
         perror("Utworzenie kolejki komunikatow");
         exit(1);
      }
   }
   else{
      elem.mtype = PUSTY;
      for (i=0; i<MAX; i++)
         if (msgsnd(msgid, &elem, sizeof(elem.mvalue), 0) == -1){
            perror("Wyslanie pustego komunikatu");
            exit(1);
         }
   }

   for (i=0; i<10000; i++){
      if (msgrcv(msgid, &elem, sizeof(elem.mvalue), PELNY, 0) == -1){
         perror("Odebranie elementu");
         exit(1);
      }
      printf("Numer: %5d   Wartosc: %5d\n", i, elem.mvalue);
      elem.mtype = PUSTY;
      if (msgsnd(msgid, &elem, sizeof(elem.mvalue), 0) == -1){
         perror("Wyslanie pustego komunikatu");
         exit(1);
      }
   }
}