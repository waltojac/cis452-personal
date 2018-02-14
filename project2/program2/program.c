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
int Searcher(int i);
void spawnChildren();
int freeMemory();
void sigHandler(int);



//variables
char first[50];
char second[50];
int numFiles = 0;
char ** fileNames;
int child_pid[10];
int ** searchPipe;
int ** statsPipe;
char ** stats;
FILE * file;

//for resetting the stdout
int saveStdout;

int main(int argc, char * argv[]){
    int i = 0;

    //save stdout
    saveStdout = dup(1);

    //get the file names
    grabNames();


    //allocate searchPipe storage array
    searchPipe = (int**)malloc(sizeof(int*)*numFiles);
    //allocate memory
    for (i = 0; i < numFiles; i++){
        searchPipe[i] = (int*)malloc(sizeof(int)*2);
    }

    //allocate statsPipe storage array
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
            printf("Search text pipe %d opened successfully.\n", i+1);
        }
    }

    //open stats pipes
    for (i = 0; i < numFiles; i++){
        if (pipe (statsPipe[i]) < 0) {
            perror ("plumbing problem");
            exit(1);
        }else{
            printf("Statistics pipe %d opened successfully.\n", i+1);
        }
    }

    //install signal handler on all processes so each will free its own memory
    signal(SIGINT, sigHandler);

    //spawn Searchers
	printf("Spawning searchers...\n");
	spawnChildren();

    //just to avoid infinite loop warning in CLion
    int flag = 1;

    //search text loop. Asks user for search string, sends to Searchers, and report results.
	while(flag){
        char text[50];

        //for output cleanliness
        sleep(1);

		//take in search string
        fflush(stdin);
        printf("\n\n--------------------------------------------");
        printf("\nPlease enter search string: \n");
        scanf("%s", text);

        //sleep(1);

        //send string to Searchers via Pipe
        for (int j = 0; j < numFiles; j++){
            dup2(searchPipe[j][WRITE], STDOUT_FILENO);
            int numBytes = (int)write(STDOUT_FILENO, (const void *)text, (size_t)strlen(text));
            fflush(stdout);
            dup2(saveStdout, 1);
            //printf("\nNumBytes written: %d\n", numBytes);
            fflush(stdout);
        }

        sleep(1);
        //read from statsPipes
        for (int j = 0; j < numFiles; j++){
            int num = (int)read(statsPipe[j][READ], (void *) stats[j], 15);
            //printf("\nNumBytes read: %d, wc: %s\n", num, stats[j]);
        }

        for (int j = 0; j < numFiles; j++){
            printf("\nWord count of \"%s\" in %s is: %s.", text, fileNames[j], stats[j]);
        }

            //report statistics
		

		//conduct another search?
	
	}

	//graceful shutdown

}

void grabNames(){

    int valid = 1;

    //accept number of files
	printf("Please enter the number of files to be searched: \n ");
    char * strNum = (char*)malloc(sizeof(char)*2);

    while(valid) {
        scanf("%s", strNum);
        numFiles = atoi(strNum);
        if ((numFiles > 0) && numFiles < 11) {
            valid = 0;
            break;
        }
        fflush(stdin);
        printf("\nPlease enter a number between 1-10.\n");
    }

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


int Searcher(int i){

    if ((file = fopen(fileNames[i], "r")) == NULL){
        printf("\nFile %s not opened successfully.\n", fileNames[i]);
        //FIXEME graceful shutdown
        exit(0);
    }

    //variables
	printf("Searcher %d spawned...\n", i+1);

    //manage search text pipe
    dup2(searchPipe[i][READ],STDIN_FILENO);
    //close pipe
    close(searchPipe[i][WRITE]);
    close(searchPipe[i][READ]);

    //close statsPipe read
    close(searchPipe[i][READ]);


    int flag = 1;
    while(flag) {
        char * text = (char *)malloc(sizeof(char)*50);

        //get search text
        fflush(stdin);
        int num = (int)read(STDIN_FILENO, (void*)text, 50);
        //printf("\nNumBytes read: %d\n", num);
        fflush(stdout);
        printf("\nChild %d searching %s for \"%s\"\n", i+1, fileNames[i], text);
        fflush(stdout);



        char * temp = (char*)malloc(sizeof(char)*50);
        int wordCount = 0;
        //read + search file
        while (fscanf(file, "%s", temp) != EOF){
            if (strcmp(temp, text) == 0 ) {
                wordCount++;
                //printf("\nTemp: %s", temp);
            }
        }
        //go back to start of the file
        rewind(file);
        //free the temp string
        free(temp);

        //convert int to string
        //help found from user2622016 on stackoverflow.com/questions/8257714/how-to-convert-an-int-to-string-in-c
        int length = snprintf(NULL, 0, "%d", wordCount);
        char * wc = (char*)malloc(length + 1);
        snprintf(wc, length + 1, "%d", wordCount);

        //send to Master via pipe
        int numBytes = (int) write(statsPipe[i][WRITE], wc, length + 1);

        //free strings
        free(wc);
        free(text);
    }
    return 0;
}

int freeMemory(){
    int i;

    //close search text pipes
    for (i = 0; i < numFiles; i++){
        close(searchPipe[i][READ]);
        close(searchPipe[i][WRITE]);

    }

    //close stats pipes
    for (i = 0; i < numFiles; i++){
        close(statsPipe[i][READ]);
        close(statsPipe[i][WRITE]);
    }

    //free search pipe memory
    for (i = 0; i < numFiles; i++){
        free(searchPipe[i]);
    }
    free(searchPipe);

    //free stats pipe memory
    for (i = 0; i < numFiles; i++){
        free(statsPipe[i]);
    }
    free(statsPipe);

    //free stats memory
    for (i = 0; i < numFiles; i++){
        free(stats[i]);
    }
    free(stats);

    //free fileNames memory
    for (i = 0; i < numFiles; i++){
        free(fileNames[i]);
    }
    free(fileNames);
    return 0;
}



void sigHandler(int sigNum){
	
	//graceful shutdown
	if (sigNum == SIGINT){
        dup2(saveStdout, STDOUT_FILENO);
        fclose(file);
        printf("\nFreeing memory...");
        freeMemory();
        printf("\nMemory freed.");
        printf("\nShutting down...\n\n");
        exit(0);
	}

}


