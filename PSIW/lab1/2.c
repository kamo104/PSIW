#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#define MAX 512

void reverse(char *tab,int size){
   // &tab[0]
   for(int i=0,j=size-1;i<(size-1)/2,j>(size-1)/2;i++,j--){
      char* temp = tab[i];
      tab[i] = tab[j];
      tab[j] = temp;
   }

}


int main(int argc, char* argv[]){
   int in;

   if ( argc < 2 )
   perror("Za malo argumentow");
      exit (1);

   in = open(argv[1], O_RDONLY);

   if ( in == -1 ) {
      perror("Blad otwarcia pliku");
      exit (1);
   }

   int size = lseek(in,0,SEEK_END );
   if(size==-1){
      perror("Blad dostepu do pliku");
      exit (1);
   }

   int num_of_chunks = size/MAX;
   int rest = size%MAX;

   char offset[MAX];
   char temp1[MAX];
   char temp2[MAX];

   int where = lseek(in, num_of_chunks*MAX, SEEK_SET );

   int got = read(in, offset, rest);


   reverse(offset,got);
   int wrote = write(in,offset,rest);


   while(num_of_chunks--){
      where = lseek(in, -num_of_chunks, SEEK_SET );


      got = read(in, temp, MAX);


      reverse(temp,got);
      wrote = write(in, temp, MAX);

   }

   exit(0);
}
