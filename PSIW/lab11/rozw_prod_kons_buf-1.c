#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>


#define BUF_SIZE 2
static volatile int buf[BUF_SIZE] = {0};
static volatile int last_position = 0;

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cnd_empty = PTHREAD_COND_INITIALIZER,
                      cnd_full  = PTHREAD_COND_INITIALIZER;


void put(int x){
   pthread_mutex_lock( &mtx );
   while ( last_position == BUF_SIZE-1 ) { // bufor pełny
      pthread_cond_wait( &cnd_empty, &mtx );
   }

   buf[++last_position] = x;

   pthread_cond_signal( &cnd_full );
   pthread_mutex_unlock( &mtx );
}

int get(){
   int x;
   pthread_mutex_lock( &mtx );

   while( last_position == -1){ // bufor pusty
      pthread_cond_wait( &cnd_full, &mtx );
   }

   x = buf[last_position--];

   pthread_cond_signal( &cnd_empty );
   pthread_mutex_unlock( &mtx );
   return x;
}

void* send1(void* p){
   put(1);
}
   
void* send2(void* p){
   put(2);
}
 
void* receive(void* p){
   int x;
   x = get();
   printf("%d\n", x); fflush(stdout);
   x = get();
   printf("%d\n", x); fflush(stdout);
   x = get();
   printf("%d\n", x); fflush(stdout);
}
 
int main(){
   pthread_t tsnd_1, tsnd_2, trcv;
   pthread_create(&tsnd_1, NULL, send1, NULL);
   pthread_create(&tsnd_2, NULL, send2, NULL);
   pthread_create(&trcv, NULL, receive, NULL);
   pthread_join(trcv, NULL);
   return 0;
}
