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
#include "storage.h"

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
    key_t shmKey = getStorageKey();
    key_t msgQKey = getKey( "./director.c", 'M' );
    key_t workersSemKey = getKey( "./worker.c", 'W' );
    key_t deliverySemKey = getKey( "./delivery.c", 'D' );

    /* create all the child processes here */
    pid_t storagePID;
    if( (storagePID = fork()) == -1 ){
        perror("storage fork error");
        exit(errno);
    }
    else if( storagePID == 0 ){
        if( execl( "./storage", "storage", shmKey, msgQKey, NULL )  == -1){
            perror("error running storage process");
            exit(errno);
        }
    }

    pid_t workers[WORKERS];
    for( int i = 0; i < WORKERS; i++ ){
        if( (workers[i] = fork()) == -1 ){
            perror("worker fork error");
            exit(errno);
        }
        else if( workers[i] == 0 ){
            if( execl( "./worker", "worker", shmKey, msgQKey, workersSemKey, deliverySemKey, NULL )  == -1){
                perror("error running worker process");
                exit(errno);
            }
        }
    }

    // TODO
    pid_t directorPID;
    if( (directorPID = fork()) == -1 ){
        perror("director fork error");
        return errno;
    }
    else if( directorPID == 0 ){
        if( execl( "./director", "director", msgQKey, NULL )  == -1){
            perror("error running director process");
            exit(errno);
        }
    }

    return 0;
}

/*
    not for storage!
*/
key_t getKey( char* path, int id ){
    key_t key = ftok( path, id );
    // if( key == -1 ){
    //     perror("Key error");
    //     return -1;
    // }
    return key;
}

key_t getStorageKey(){
    if( !fileExists(STORAGE_FILENAME) ){
        FILE *ptr;
        ptr = fopen(STORAGE_FILENAME, "w");
        if( ptr == NULL ){
            perror("Storage file missing could not create a new file");
            exit -2;
        }
        fclose(ptr);
    }
    
    char *tmp = malloc(strlen(STORAGE_FILENAME) + 3);
    if( tmp == NULL ){
        perror("memory allocation error");
        exit(-1);
    }
    strcpy( tmp, "./" );
    strcat( tmp, STORAGE_FILENAME );
    
    key_t key;
    if( (key = getKey(tmp, 'S')) == -1 ){
        free(tmp);
        exit(-1);
    }
    free(tmp);
    return key;
}

int storageSetup(key_t shmKey){
    int sizeBytes = STORAGE_SIZE + 6 * sizeof(int);
    int shmId = shmget( shmKey, sizeBytes, IPC_CREAT|200 );
    if( shmId == -1 ){
        perror("Shared memory allocation error");
        exit( errno );
    }

    void *shm;
    shm = shmat( shmId, NULL, 0 );

    return 0;
}