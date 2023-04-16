#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int fdp0[2],fdp1[2],fdp2[2];


int main(){

	int p0 = pipe(fdp0);
	int p1 = pipe(fdp1);
	int p2 = pipe(fdp2);

	
	if(p0==-1||p1==-1||p2==-1){
		perror("blad tworzenia pipe");
		exit(1);
	}

	close(0);

	//first exec
	switch(fork()){
		case(-1):{
			perror("blad tworzenia procesu potomnego");
			break;
		}
		case(0):{ //child
			close(fdp0[0]);
			// close(fdp0[1]);

			close(fdp1[0]);
			close(fdp1[1]);

			close(fdp2[0]);
			close(fdp2[1]);


			dup2(fdp0[1],1);

			execlp("ps","ps","-e","-o","uname",NULL); // ps -e -o uname
			exit(1);
		}
		default:{ //parent
		}
	}


	// second exec
	switch(fork()){
		case(-1):{
			perror("blad tworzenia procesu potomnego");
			break;
		}
		case(0):{ //child	
			// close(fdp0[0]);
			close(fdp0[1]);

			close(fdp1[0]);
			// close(fdp1[1]);

			close(fdp2[0]);
			close(fdp2[1]);


			dup2(fdp0[0],0);
			dup2(fdp1[1],1);

			execlp("sort","sort",NULL); // ps -e -o uname
			exit(1);
		}
		default:{ //parent
		}
	}


	//third exec
	switch(fork()){
		case(-1):{
			perror("blad tworzenia procesu potomnego");
			break;
		}
		case(0):{ //child		
			close(fdp0[0]);
			close(fdp0[1]);

			// close(fdp1[0]);
			close(fdp1[1]);

			close(fdp2[0]);
			// close(fdp2[1]);


			dup2(fdp1[0],0);
			dup2(fdp2[1],1);

			execlp("uniq","uniq","-c",NULL); // ps -e -o uname
			exit(1);
		}
		default:{ //parent
			//fourth exec
			close(fdp0[0]);
			close(fdp0[1]);

			close(fdp1[0]);
			close(fdp1[1]);

			// close(fdp2[0]);
			close(fdp2[1]);


			dup2(fdp2[0],0);


			execlp("sort","sort","-n",NULL); // ps -e -o uname
			exit(1);
		}
	}



}
