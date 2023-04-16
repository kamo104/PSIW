#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define MAX_SIZE 512

char commands[MAX_SIZE][MAX_SIZE];
char *args[MAX_SIZE][MAX_SIZE];


int main(int argc, char* argv[]){
    close(1);
    int fd = creat("log",0600);
    dup2(fd,2);
    if(fd==-1){
        perror("blad tworzenia pliku");
        exit(1);
    }
    else if(fd!=1){
        dup2(fd,1);
    }
    
    int num_of_commands=0;
    int last_args=0;
    int arg_place=0;
    for(int i=1;i<argc;i++){
        if(argv[i][0]!='-' && argv[i][0]!='\"'){
            strcpy(commands[num_of_commands], argv[i]);

            arg_place=num_of_commands;
            num_of_commands++;

            args[arg_place][0] = strdup(argv[i]);
            last_args=1;
            
        }
        else{
            args[arg_place][last_args++] = strdup(argv[i]);
            args[arg_place][last_args] = NULL; // last arg has to be a nullptr
        }
    }
    

    for(int i=0;i<num_of_commands;i++){
        switch(fork()){
            case(-1):{
                perror("blad tworzenia procesu potomnego");
                exit(1);
            }
            case(0):{
                execvp(commands[i],args[i]);
                perror("blad egzekucji programu");
                exit(1);
            }
        }
    }
    while(wait(NULL) > 0);

    exit (0);
}
