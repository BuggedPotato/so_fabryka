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

key_t getStorageKey();

int main(int argc, char *argv[]){

    int storageSize = STORAGE_SIZE;
    if( argc > 1 )
        storageSize = atoi(argv[1]);
    if( storageSize <= 0 ){
        perror( "Invalid storage size" );
        errno = EINVAL;
        exit(errno);
    }
    // key_t shmKey = getStorageKey();
    // key_t msgQKey = getKey( "./director.c", 'M' );
    // key_t workersSemKey = getKey( "./worker.c", 'W' );
    // key_t deliverySemKey = getKey( "./delivery.c", 'D' );

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
        if( execl( "./director", "director", NULL )  == -1){
            perror("error running director process");
            exit(errno);
        }
    }

    say( "Done! Shutting down..." );

    return 0;
}

