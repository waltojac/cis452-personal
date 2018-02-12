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

//variables
char first[50];
char second[50];

char ** fileNames;


int main(int argc, char * argv[]){

	//create fileNames array
	fileNames = (char**)malloc(sizeof(char*)*500);
	
	

	int i = 0;
	//allocate memory
	for (i = 0; i < 11; i++){
		fileNames[i] = (char*)malloc(sizeof(char*));
	}

	//get the filenames
	grabNames();

	int child1_pid = fork();
      if(child1_pid < 0) {
           perror("An error occurred while forking");
           exit(1);
       } else if(child1_pid == 0) {    //Child Process
	     

		 } else {    // parent


		 }

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
	int numFiles = 0;

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

	//spawn Searchers
	printf("Spawning searchers...\n");

}


void* Searcher(void* args){
	
	//open file
	

	//read + search file
	

	//send to Master via pipe
	return args;
}



void sigHandler(int sigNum){
	
	//graceful shutdown
	if (sigNum == SIGINT){
		printf("Shutting down...");
		exit(0);
	}

}


