#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

#define READ 0
#define WRITE 1

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
int ** searchPipe;
int ** statsPipe;
char ** stats;



int main(int argc, char * argv[]){
    int i = 0;

    //get the file names
    grabNames();
    printf("%s, %s\n", fileNames[0], fileNames[1]);


    //allocate pipe searchPipe array
    searchPipe = (int**)malloc(sizeof(int*)*numFiles);
    //allocate memory
    for (i = 0; i < numFiles; i++){
        searchPipe[i] = (int*)malloc(sizeof(int)*2);
    }

    //allocate pipe statsPipe array
    statsPipe = (int**)malloc(sizeof(int*)*numFiles);
    //allocate memory
    for (i = 0; i < numFiles; i++){
        statsPipe[i] = (int*)malloc(sizeof(int)*2);
    }

    //create stats array
    stats = (char**)malloc(sizeof(char*)*500);
    //allocate memory
    for (i = 0; i < numFiles; i++){
        stats[i] = (char*)malloc(sizeof(char*));
    }


    //open search text pipes
    for (i = 0; i < numFiles; i++){
        if (pipe (searchPipe[i]) < 0) {
            perror ("plumbing problem");
            exit(1);
        }else{
            printf("Pipe %d opened successfully.\n", i);
        }
    }

    //open stats pipes
    for (i = 0; i < numFiles; i++){
        if (pipe (statsPipe[i]) < 0) {
            perror ("plumbing problem");
            exit(1);
        }else{
            printf("Pipe %d opened successfully.\n", i);
        }
    }

	//spawn Searchers
	printf("Spawning searchers...\n");
	spawnChildren();

    int flag = 1;

    //loop
	while(flag){
        char text[50];


        //for output cleanliness
        sleep(1);

		//take in search string
        printf("Please enter search string: \n");
        scanf("%s", text);

        //send string to Searchers via Pipe
        for (int j = 0; j < numFiles; j++){
            int numBytes = (int)write(searchPipe[j][WRITE], (const void *)text, (size_t)strlen(text)+1);
            close(searchPipe[j][READ]);
            printf("\nNumBytes written: %d", numBytes);
            fflush(stdout);

        }


        //read from statsPipes
        for (int j = 0; j < numFiles; j++){
            int num = (int)read(statsPipe[j][READ], (void *) stats[j], (size_t)  sizeof (text));
            close(statsPipe[j][READ]);
            printf("\nNumBytes read: %d, wc: %s", num, stats[j]);
        }

        for (int j = 0; j < numFiles; j++){
            printf("\nWord count of %s in %s is: %s.\n", text, fileNames[j], stats[j]);
        }

            //report statistics
		

		//conduct another search?
	
	}

	//graceful shutdown

}

void grabNames(){

	//accept number of files
	printf("Please enter the number of files to be searched: \n ");
	scanf("%d", &numFiles);

    //create fileNames array
    fileNames = (char**)malloc(sizeof(char*)*500);

    //allocate memory
    int i = 0;
    for (i = 0; i < numFiles; i++){
        fileNames[i] = (char*)malloc(sizeof(char*));
    }


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
    //variables
	printf("Searcher %d spawned...\n", i);

    //manage pipe
    dup2(searchPipe[i][READ],STDIN_FILENO);
    //close pipe
    close(searchPipe[i][WRITE]);
    close(searchPipe[i][READ]);

    int flag = 1;
    while(flag) {
        char text[50];
        //get search text
        int num = (int) read(STDIN_FILENO, (void *) text, (size_t) sizeof(text));
        fflush(STDIN_FILENO);


        printf("\nChild %d received word %s", i, text);

        //open file

        //read + search file


        //send to Master via pipe
        int numBytes = (int) write(statsPipe[i][WRITE], "1", 1);


    }
    exit(0);
}



void sigHandler(int sigNum){
	
	//graceful shutdown
	if (sigNum == SIGINT){
		printf("Shutting down...");
		exit(0);
	}

}


