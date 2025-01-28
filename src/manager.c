#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<string.h>
#include<sys/shm.h>
#include<errno.h>
#include "../include/constants.h"
#include "../include/utils.h"

FILE *LOG_FILE;

// fake signal handler for pause
void foo(int sig){
    return;
}

int main(int argc, char *argv[]){
    signal(SIGCONT, foo);

    // log filename creation
    char fileName[64];
    time_t tmp;
    time(&tmp);
    strftime( fileName, sizeof(fileName), "./logs/log_%d-%m-%Y_%X.log", localtime(&tmp) );

    LOG_FILE = fopen(fileName, "w");
    if( LOG_FILE == NULL ){
        perror( "Could not open log file" );
        exit(errno);
    }
    // replaces stderr with log file
    if( dup2( fileno(LOG_FILE), fileno(stderr) ) == -1 ){
        perror( "dup error" );
        fclose(LOG_FILE);
        exit(EXIT_FAILURE);
    }

   
    pid_t storagePID;
    if( (storagePID = fork()) == -1 ){
        perror("storage fork error");
        exit(errno);
    }
    else if( storagePID == 0 ){
        if( execl( "./storage", "storage", NULL )  == -1){
            perror("error running storage process");
            exit(errno);
        }
    }
    
    say( "Awaiting storage setup..." );
    pause();
    say( "Continuing" );

    pid_t workersPID[WORKERS];
    for( int i = 0; i < WORKERS; i++ ){
        if( (workersPID[i] = fork()) == -1 ){
            perror("worker fork error");
            exit(errno);
        }
        else if( workersPID[i] == 0 ){
            if( execl( "./worker", "worker", NULL )  == -1){
                perror("error running worker process");
                exit(errno);
            }
        }
    }

    pid_t deliveriesPID[DELIVERIES];
    int foo[3] = { SIZE_X, SIZE_Y, SIZE_Z };
    char els[3][2];
    for( int i = 0; i < DELIVERIES; i++ ){
        if( (deliveriesPID[i] = fork()) == -1 ){
            perror("delivery fork error");
            exit(errno);
        }
        else if( deliveriesPID[i] == 0 ){
            sprintf( els[i], "%d", foo[i] );
            if( execl( "./delivery", "delivery", els[i], NULL )  == -1){
                perror("error running delivery process");
                exit(errno);
            }
        }
    }

    // switch to director process to keep terminal input
    if( execl( "./director", "director", NULL )  == -1){
        perror("error running director process");
        exit(errno);
    }
    say( "Done! Shutting down..." );

    return 0;
}

