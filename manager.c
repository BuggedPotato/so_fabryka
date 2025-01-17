#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<string.h>
#include<sys/shm.h>
#include<errno.h>
#include "constants.h"
#include "utils.h"


int main(int argc, char *argv[]){

    int storageSize = STORAGE_SIZE;
    if( argc > 1 )
        storageSize = atoi(argv[1]);
    if( storageSize <= 0 ){
        perror( "Invalid storage size" );
        errno = EINVAL;
        exit(errno);
    }

    /* create all the child processes here */
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

    // pid_t workers[WORKERS];
    // for( int i = 0; i < WORKERS; i++ ){
    //     if( (workers[i] = fork()) == -1 ){
    //         perror("worker fork error");
    //         exit(errno);
    //     }
    //     else if( workers[i] == 0 ){
    //         if( execl( "./worker", "worker", shmKey, msgQKey, workersSemKey, deliverySemKey, NULL )  == -1){
    //             perror("error running worker process");
    //             exit(errno);
    //         }
    //     }
    // }

    // TODO
    pid_t directorPID;
    if( (directorPID = fork()) == -1 ){
        perror("director fork error");
        exit(errno);
    }
    else if( directorPID == 0 ){
        char tmp[WORKERS+1][6];
        sprintf( tmp[0], "%d", storagePID );
        // sprintf( tmp[1], "%d", workersPID[] );
        // for( int i = 0; i < WORKERS; i++ ){
        //     sprintf( tmp[i+1], "%d", workersPID[i] );
        // }
        if( execl( "./director", "director", tmp[0], NULL )  == -1){
            perror("error running director process");
            exit(errno);
        }
    }

    say( "Done! Shutting down..." );

    return 0;
}

