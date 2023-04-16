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
   int in, out;

   if ( argc < 3 )
   perror("Za malo argumentow");
      exit (1);

   in = open(argv[1], O_RDONLY);
   out = creat(argv[2], 0600);

   if ( in == -1 || out == -1) {
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
   char temp[MAX];

   int where = lseek(in, num_of_chunks*MAX, SEEK_SET );
   if(where==-1){
      perror("Blad dostepu do pliku");
      exit (1);
   }

   int got = read(in, offset, rest);
   if(got==-1){
      perror("Blad czytania z pliku");
      exit (1);
   }

   reverse(offset,got);
   int wrote = write(out,offset,rest);
   if(wrote==-1){
      perror("Blad zapisu do pliku");
      exit (1);
   }

   while(num_of_chunks--){
      where = lseek(in, num_of_chunks*MAX, SEEK_SET );
      if(where==-1){
         perror("Blad dostepu do pliku");
         exit (1);
      }

      got = read(in, temp, MAX);
      if(got==-1){
         perror("Blad czytania z pliku");
         exit (1);
      }

      reverse(temp,got);
      wrote = write(out, temp, MAX);
      if(wrote==-1){
         perror("Blad zapisu do pliku");
         exit (1);
      }
   }

   exit(0);
}
