#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<string.h>
#include<sys/shm.h>
#include<errno.h>
#include "constants.h"
#include "utils.h"

// fake signal handler for pause
void foo(int sig){
    return;
}

int main(int argc, char *argv[]){
    signal(SIGCONT, foo);
   
    pid_t storagePID;
    // char tmp[6];
    // sprintf( tmp, "%d", directorPID );
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
    if( execl( "./director", "director", NULL )  == -1){
        perror("error running director process");
        exit(errno);
    }
    say( "Done! Shutting down..." );

    return 0;
}

