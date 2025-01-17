#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/sem.h>
#include<sys/msg.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<errno.h>
#include "constants.h"
#include "types.h"
#include "utils.h"

int createStorage(key_t key, int size);
int storageSetup(int shmId, char **shmAddr, int size);
int getOptimalSegmentSize(int M);
int deleteStorage( int shmId, void *shmAddr );
int loadStorageFile( char *fileName, char *dest, char *end );

int semaphoresSetup( int semId );
int deleteSemaphores( int semId );

pid_t PID;

int main(int argc, char *argv[]){
    PID = getpid();
    key_t shmKey = getKey( STORAGE_KEY_STR, STORAGE_KEY_CHAR );
    char *shmAddr = NULL;
    int shmId = createStorage( shmKey, STORAGE_TOTAL_SIZE );
    storageSegment segments[3];
    // maybe unnecessary if?
    if( storageSetup(shmId, &shmAddr, STORAGE_TOTAL_SIZE) ){
        perror( "error building storage structure" );
        exit(EXIT_FAILURE);
    }

    key_t semKey = getKey( SEM_KEY_STR, SEM_KEY_CHAR );
    int semId = getSemaphores( semKey, 2, 0700 );
    semaphoresSetup( semId );

    key_t msgQKey = getKey( MSGQ_KEY_STRING, MSGQ_KEY_CHAR );
    int msgQId = getMessageQueue( msgQKey, 0700 );
    message msg;
    say("Entering standby mode - awaiting messages...");
    if( msgrcv( msgQId, (void *)&msg, MSG_TEXT_SIZE, MESSAGES_STORAGE, 0 ) == -1 ){
        perror("error receiving a message");
        exit(EXIT_FAILURE);
    }
    say( "Got a message!" );

    deleteStorage( shmId, (void *)shmAddr );
    deleteSemaphores(semId);
    say("Shutting down");

    return 0;
}
/*
    creates shm of 'size' elements + delivery and recieve value points
    size = M
*/
int createStorage(key_t key, int size){
    
    int shmId = shmget( key, size, IPC_CREAT|0700 );
    if( shmId == -1 ){
        perror("Shared memory allocation error");
        exit( errno );
    }
    // TODO saved file write
    return shmId;
}

int storageSetup(int shmId, char **shmAddr, int size){
    void *tmpPtr;
    tmpPtr = shmat( shmId, NULL, 0 );
    if( tmpPtr == (void *)-1 ){
        perror("Error attaching shared memory");
        if( shmctl( shmId, IPC_RMID, NULL ) == -1 ){
            perror( "error deleting shared memory" );
        }
        exit(errno);
    }
    char *shm = (char *)tmpPtr;
    *shmAddr = shm;

    loadStorageFile( STORAGE_FILENAME, *shmAddr, *shmAddr + size );

    return 0;
}

int deleteStorage( int shmId, void *shmAddr ){
    if( shmdt( shmAddr ) == -1 ){
        perror( "error detaching shared memory" );
        exit(errno);
    }
    if( shmctl( shmId, IPC_RMID, NULL ) == -1 ){
        perror( "error deleting shared memory" );
        exit( errno );
    }
        // TODO save to file
    #if DEBUG
        say("Successfully deleted storage");
    #endif
    return 0;
}

int semaphoresSetup( int semId ){
    if( semctl( semId, SEM_WORKERS, SETVAL, 1 ) == -1 || semctl( semId, SEM_DELIVERY, SETVAL, 1 ) == -1 ){
        perror("cannot set semaphores init value");
        exit(errno);
    }
    return 0;
}

int deleteSemaphores( int semId ){
    if( semctl( semId, 0, IPC_RMID ) == -1 ){
        perror("cannot delete semaphore set");
        exit(errno);
    }
    #if DEBUG
        say("Successfully deleted semaphore set");
    #endif
    return 0;
}

int loadStorageFile( char *fileName, char *dest, char *end ){
    say("Loading storage file");
    FILE* file = NULL;
    if( (file = fopen( fileName, "r" )) == NULL ){
        warning("No saved storage file found! Starting clean");
        memset( dest, 0, end - dest );
        return -1;
    }
    char buffer[256];
    int res = 0;
    int shift = 0;
    while( (res = fread( buffer, sizeof(char), 256, file )) != 0 ){
        shift += res;
        if( dest + shift >= end ){
            warning("Input file too long wiping storage");
            fclose(file);
            memset( dest, 0, end - dest );
            return 1;
        }
        memcpy( dest + shift, buffer, res * sizeof(char) );
    }
    fclose( file );
    say("File loaded successfully");
    return 0;
}
