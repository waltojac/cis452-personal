#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

//function declarations
void grabNames();
void Searcher(int i);
void spawnChildren();


//variables
char first[50];
char second[50];
int numFiles = 0;
char ** fileNames;
int child_pid[10];

int main(int argc, char * argv[]){

	//create fileNames array
	fileNames = (char**)malloc(sizeof(char*)*500);
	
	//allocate memory
	int i = 0;
	for (i = 0; i < 11; i++){
		fileNames[i] = (char*)malloc(sizeof(char*));
	}

	//get the filenames
	grabNames();
	printf("%s, %s\n", fileNames[0], fileNames[1]);

	//spawn Searchers
	printf("Spawning searchers...\n");
	spawnChildren();

	//loop 
	while(1){
		
		//take in search string
		


		//send string to Searchers via Pipe
		

		//report statistics
		

		//conduct another search?
	
		
	}

	//graceful shutdown

}

void grabNames(){

	//accept number of files
	printf("Please enter the number of files to be searched: \n ");
	scanf("%d", &numFiles);
	
	int i = 0;
	for(i = 0; i < numFiles; i++){
		//accept  file names
		printf("Please enter filename %d: \n ", i);
		fflush(stdin);
		scanf("%s", fileNames[i]);
	}
}

void spawnChildren(){
	int i;
	for(i = 0; i < numFiles; i++){
		child_pid[i] = fork();
      	if(child_pid[i] < 0) {
       	    perror("An error occurred while forking");
        	exit(1);
       	} else if(child_pid[i] == 0) {    //Child Process
	     	Searcher(i);
			break;
		 }
	}
	
}


void Searcher(int i){
	printf("Child %d searching %s...\n", i, fileNames[i]);
	//open file
	

	//read + search file
	

	//send to Master via pipe
	

	exit(0);
}



void sigHandler(int sigNum){
	
	//graceful shutdown
	if (sigNum == SIGINT){
		printf("Shutting down...");
		exit(0);
	}

}


