#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/sem.h>
#include<sys/msg.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<errno.h>
#include "constants.h"
#include "types.h"
#include "utils.h"

int getStorage(key_t key, int size);
int storageSetup(int shmId, char **shmAddr, storageSegment *storageSegments);
int getOptimalSegmentSize(int M);
int deleteStorage( int shmId, void *shmAddr );

int semaphoresSetup( int semId );
int deleteSemaphores( int semId );

pid_t PID;

int main(int argc, char *argv[]){
    PID = getpid();
    key_t shmKey = getKey( "./storage.c", 'S' );
    char *shmAddr = NULL;
    int shmId = getStorage( shmKey, STORAGE_SIZE );
    storageSegment segments[3];
    // maybe unnecessary if?
    if( storageSetup(shmId, &shmAddr, segments) ){
        perror( "error building storage structure" );
        exit(EXIT_FAILURE);
    }

    key_t semKey = getKey( "./delivery.c", 'D' );
    int semId = getSemaphores( semKey, 2, 0700 );
    semaphoresSetup( semId );

    key_t msgQKey = getKey( "./director.c", 'M' );
    int msgQId = getMessageQueue( msgQKey, 0700 );
    message msg;
    say("Entering standby mode - awaiting messages...");
    if( msgrcv( msgQId, (void *)&msg, MSG_TEXT_SIZE, MESSAGES_STORAGE, 0 ) == -1 ){
        perror("error receiving a message");
        exit(EXIT_FAILURE);
    }
    say( "Got a message!" );

    say( "Sleep now..." );
    sleep(5);

    deleteStorage( shmId, (void *)shmAddr );
    deleteSemaphores(semId);

    return 0;
}
/*
    creates shm of 'size' elements + delivery and recieve value points
    size = M
*/
int getStorage(key_t key, int size){
    
    int sizeBytes = size + 6 * sizeof(int);
    int shmId = shmget( key, sizeBytes, IPC_CREAT|0700 );
    if( shmId == -1 ){
        perror("Shared memory allocation error");
        exit( errno );
    }
    // TODO saved file write
    return shmId;
}

int storageSetup(int shmId, char **shmAddr, storageSegment *storageSegments){
    void *tmpPtr;
    tmpPtr = shmat( shmId, NULL, 0 );
    if( tmpPtr == (void *)-1 ){
        perror("Error attaching shared memory");
        if( shmctl( shmId, IPC_RMID, NULL ) == -1 ){
            perror( "error deleting shared memory" );
            exit( errno );
        }
        exit(errno);
    }
    char *shm = (char *)tmpPtr;
    *shmAddr = shm;

    int baseSize = getOptimalSegmentSize( STORAGE_SIZE );
    storageSegments[0].start = shm;
    storageSegments[0].end = storageSegments[0].start + baseSize;
    for( int i = 1; i < 3; i++ ){
        storageSegments[i].start = storageSegments[i-1].end;
        storageSegments[i].end = storageSegments[i].start + (i+1) * baseSize;
    }

    storageSegments[0].read = (int *)storageSegments[2].end;
    storageSegments[0].write = storageSegments[0].read + 1;
    for( int i = 1; i < 3; i++ ){
        storageSegments[i].read = storageSegments[i-1].write + 1;
        storageSegments[i].write = storageSegments[i].read + 1;
    }

    // TODO read saved file

    #if DEBUG
        /*for( int i = 0; i < 3; i++ ){
            printf( "storage int[%d]: %p - %p\n", i, storageSegments[i].start, storageSegments[i].end );
            printf( "access     [%d]: r -%p, w - %p\n", i, storageSegments[i].read, storageSegments[i].write );
        }*/
    #endif

    return 0;
}

/*
    M elements total
*/
int getOptimalSegmentSize(int M){
    return M / (SIZE_X + SIZE_Y + SIZE_Z);
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
