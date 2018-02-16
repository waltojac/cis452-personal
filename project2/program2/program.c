/*
 * Title: File Searcher
 * Author: Jacob Walton
 * Date: February 15, 2018
 * Description: Searchers given text files for search strings that the user inputs.
 * Can search up to 10 different files for the same word at the same time. This could
 * be adjusted by increasing the size of some of the arrays. Uses pipes for communication
 * between Master process and its searchers. Uses signal IPCS for shutting down (ctrl + C).
 * Written for cis452 taught by Professor Wolfe.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>

#define READ 0
#define WRITE 1

//function declarations
void grabNames();

void Searcher(int i, int parentPid);

void spawnChildren();

int freeMemory();

void sigHandlerMain(int);

void sigHandlerSearcher(int);

int kill(pid_t pid, int sig);

//variables
int numFiles = 0;
char **fileNames;
int child_pid[10];
int **searchPipe;
int **statsPipe;
char **stats;
FILE *file;
char *intext;
int flag = 0;
int fileFlag = 0;

//for resetting the stdout
int saveStdout;

int main(int argc, char *argv[]) {
    int i = 0;

    //install crtl c signal handler (will be overridden by children)
    signal(SIGINT, sigHandlerMain);

    //save stdout
    saveStdout = dup(1);

    //get the file names
    grabNames();
    flag = 1;

    //allocate searchPipe storage array
    searchPipe = (int **) malloc(sizeof(int *) * numFiles);
    //allocate memory
    for (i = 0; i < numFiles; i++) {
        searchPipe[i] = (int *) malloc(sizeof(int) * 2);
    }

    //allocate statsPipe storage array
    statsPipe = (int **) malloc(sizeof(int *) * numFiles);
    //allocate memory
    for (i = 0; i < numFiles; i++) {
        statsPipe[i] = (int *) malloc(sizeof(int) * 2);
    }

    //create stats array
    stats = (char **) malloc(sizeof(char *) * 500);
    //allocate memory
    for (i = 0; i < numFiles; i++) {
        stats[i] = (char *) malloc(sizeof(char *));
    }

    //open search text pipes
    for (i = 0; i < numFiles; i++) {
        if (pipe(searchPipe[i]) < 0) {
            perror("plumbing problem");
            exit(1);
        } else {
            printf("Search text pipe %d opened successfully.\n", i + 1);
        }
    }

    //open stats pipes
    for (i = 0; i < numFiles; i++) {
        if (pipe(statsPipe[i]) < 0) {
            perror("plumbing problem");
            exit(1);
        } else {
            printf("Statistics pipe %d opened successfully.\n", i + 1);
        }
    }



    //spawn Searchers
    printf("Spawning searchers...\n");
    spawnChildren();

    //install child error signal handler for parent
    signal(SIGUSR1, sigHandlerMain);


    //just to avoid infinite loop warning in CLion
    int flag = 1;

    //search text loop. Asks user for search string, sends to Searchers, and report results. Repeat.
    while (flag) {
        //input search string field
        intext = (char *) malloc(sizeof(char) * 50);

        //for output cleanliness
        sleep(1);

        //take in search string
        fflush(stdin);
        printf("\n\nPlease enter search string: ");
        scanf("%s", intext);
        printf("\nSending search string to searchers...");
        fflush(stdout);

        //send string to Searchers via Pipe
        for (int j = 0; j < numFiles; j++) {
            dup2(searchPipe[j][WRITE], STDOUT_FILENO);
            write(STDOUT_FILENO, (const void *) intext, 50);
            fflush(stdout);
            dup2(saveStdout, 1);
            //printf("\nNumBytes written: %d\n", numBytes);                 for testing
            fflush(stdout);
        }


        //read from statsPipes
        for (int j = 0; j < numFiles; j++) {
            read(statsPipe[j][READ], (void *) stats[j], 15);
            //printf("\nNumBytes read: %d, wc: %s\n", num, stats[j]);       for testing
        }

        printf("\n\n--------------------------------------------\n");
        printf("     WORD COUNTS RECEIVED BY MASTER     \n");
        printf("--------------------------------------------");
        fflush(stdout);

        //print all the stats
        for (int j = 0; j < numFiles; j++) {
            printf("\nWord count of \"%s\" in %s is: %s.", intext, fileNames[j], stats[j]);
        }

        //free the input string
        free(intext);

        //repeat until SIGINT
    }
}

/*
 * Function to gather and store the number of files and all the filenames
 */
void grabNames() {

    int valid = 1;

    //accept number of files
    printf("Please enter the number of files to be searched: \n ");
    char *strNum = (char *) malloc(sizeof(char) * 2);

    //loop to make sure a valid number is entered. 10 is the limit.
    while (valid) {
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
    fileNames = (char **) malloc(sizeof(char *) * 500);

    //allocate memory
    int i = 0;
    for (i = 0; i < numFiles; i++) {
        fileNames[i] = (char *) malloc(sizeof(char *));
    }

    //loop to get all filenames. Does not check filenames for validity here, but rather in searcher functions.
    for (i = 0; i < numFiles; i++) {
        //accept  file names
        printf("Please enter filename %d: \n ", i);
        fflush(stdin);
        scanf("%s", fileNames[i]);
    }
}

/*
 * Function to spawn all the required searcher children.
 */
void spawnChildren() {
    int i;

    //used for signalling to parent that error has occured
    int parentPid = getpid();

    //loop to fork all the children
    for (i = 0; i < numFiles; i++) {
        child_pid[i] = fork();
        if (child_pid[i] < 0) {
            perror("An error occurred while forking");
            exit(1);
        } else if (child_pid[i] == 0) {    //Child Process
            //children live and die in this function. The break will never be reached.
            Searcher(i, parentPid);
            break;
        }
    }

}

/*
 * Function where the children live and die. They open their filename, read from the pipe,
 * search the file for the search string and record the word count, and send word count
 * back to parent via another pipe.
 */
void Searcher(int i, int parentPid) {
    printf("Searcher %d spawned...\n", i + 1);

    //override signal handler for searchers so they don't do mainSigHandler
    signal(SIGINT, sigHandlerSearcher);

    //try to open the file
    if ((file = fopen(fileNames[i], "r")) == NULL) {
        printf("\nFile %s not opened successfully.\n", fileNames[i]);
		fileFlag = 1;
        //let parent know so it can shut down other children
        kill(parentPid, SIGUSR1);

        //shutdown itself just in case parent didn't get to it yet.
        raise(SIGINT);
    }

    printf("\nFile \"%s\" opened successfully.", fileNames[i]);
    fflush(stdout);

    //manage search text pipe
    dup2(searchPipe[i][READ], STDIN_FILENO);
    //close search pipe
    close(searchPipe[i][WRITE]);
    close(searchPipe[i][READ]);

    //close statsPipe read end
    close(searchPipe[i][READ]);


    int flag = 1;
    while (flag) {
        char *text = (char *) malloc(sizeof(char) * 50);

        //get search text from pipe
        fflush(stdin);
        read(STDIN_FILENO, (void *) text, 50);
        fflush(stdout);
        printf("\nSearcher %d searching %s for \"%s\"", i + 1, fileNames[i], text);
        fflush(stdout);

        //read + search file for the search text
        char *temp = (char *) malloc(sizeof(char) * 50);
        int wordCount = 0;
        while (fscanf(file, "%s", temp) != EOF) {
            if (strcmp(temp, text) == 0) {
                wordCount++;
            }
        }

        //go back to start of the file
        rewind(file);
        //free the temp string
        free(temp);

        //convert int to string
        //help found from user2622016 on stackoverflow.com/questions/8257714/how-to-convert-an-int-to-string-in-c
        int length = snprintf(NULL, 0, "%d", wordCount);
        char *wc = (char *) malloc(length + 1);
        snprintf(wc, length + 1, "%d", wordCount);

        //send word count to Master via pipe
        write(statsPipe[i][WRITE], wc, length + 1);

        //free strings
        free(wc);
        free(text);

        //repeat
    }
}


/*
 * Function to close all pipes and free all memory. Called by searchers and Master
 * because each searcher is aware of the full arrays of pipes, filenames, and stats.
 *
 */
int freeMemory() {
    int i;

    printf("\nClosing pipes...");

    //close search text pipes
    for (i = 0; i < numFiles; i++) {
        close(searchPipe[i][READ]);
        close(searchPipe[i][WRITE]);
    }

    //close stats pipes
    for (i = 0; i < numFiles; i++) {
        close(statsPipe[i][READ]);
        close(statsPipe[i][WRITE]);
    }
    printf("\nPipes closed.");


    printf("\nFreeing memory...");
    //free search pipe memory
    for (i = 0; i < numFiles; i++) {
        free(searchPipe[i]);
    }
    free(searchPipe);

    //free stats pipe memory
    for (i = 0; i < numFiles; i++) {
        free(statsPipe[i]);
    }
    free(statsPipe);

    //free stats memory
    for (i = 0; i < numFiles; i++) {
        free(stats[i]);
    }
    free(stats);

    //free fileNames memory
    for (i = 0; i < numFiles; i++) {
        free(fileNames[i]);
    }
    free(fileNames);

    printf("\nMemory freed.");
    fflush(stdout);

    return 0;
}


/*
 * Function to handle signal IPCS for the master.
 *
 * Handles SIGINT or SIGUSR1.
 */
void sigHandlerMain(int sigNum) {

    //graceful shutdown
    if (sigNum == SIGINT) {
        if (flag == 0) {
            printf("\nMaster shut down.\n\n");
            fflush(stdout);
            exit(0);
        } else {
            printf("\nMaster starting graceful shutdown.");
            free(intext);
            freeMemory();
            printf("\nMaster shut down.\n\n");
            fflush(stdout);
            exit(0);
        }
    }
        //for when a file is not opened successfully, Master needs to shut down all other children
    else if (sigNum == SIGUSR1) {

        //shutdown children
        printf("\nMaster is killing other searchers if applicable...\n");
        for (int i = 0; i < numFiles; i++) {
            kill(child_pid[i], SIGINT);
        }
        printf("\nMaster starting graceful shutdown.");
        free(intext);
        freeMemory();
        printf("\nMaster shut down.\n\n");
        fflush(stdout);
        exit(0);

    }

}

/*
 * Function to handle signal IPCS for searchers.
 *
 * Handles SIGINT
 */
<<<<<<< HEAD
void sigHandlerSearcher(int sigNum) {

=======
void sigHandlerSearcher(int sigNum){
>>>>>>> 8ea3f79a9f4aa10d0dbd0676e2917f08017c7165
    //graceful shutdown
    if (sigNum == SIGINT) {
        //change stdout back to normal
        dup2(saveStdout, STDOUT_FILENO);
        printf("\nSearcher starting graceful shutdown.");
		if (fileFlag == 0){
        	fclose(file);
		}
        freeMemory();
        printf("\nSearcher shut down.\n\n");
        fflush(stdout);
        exit(0);
    }

}


