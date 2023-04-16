#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

static volatile int x=0;
static volatile int y=10;

void fun(){
   y = 1/x;
}

void f(int signum){
   printf("printek \n"); x++;
   fun();
}

int main(){
   
   signal(SIGFPE,f);
   fun();
   printf("%d\n",y);
}