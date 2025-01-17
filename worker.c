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
#include "semaphores.h"

pid_t PID;
int STORAGE_EXISTS = 1;
int getMaterials( int semId, storageSegment *storage );
int work();

int main(int argc, char *argv[]){
    PID = getpid();

    say("Started");
    key_t shmKey = getKey( STORAGE_KEY_STR, STORAGE_KEY_CHAR );
    int shmId = getStorage( shmKey );
    char *shmAddr = attachStorage(shmId);
    storageSegment storage[3];
    getStorageSegments( shmAddr, storage );
    say("Worker successfully attached storage");

    key_t semKey = getKey( SEM_KEY_STR, SEM_KEY_CHAR );
    int semId = getSemaphores( semKey, 2, 0600 );
    say("Worker successfully attached semaphores");

    key_t msgQKey = getKey( MSGQ_KEY_STRING, MSGQ_KEY_CHAR );
    int msgQId = getMessageQueue( msgQKey, 0700 );
    say("Worker successfully attached message queue");

    message msg;

    while(STORAGE_EXISTS){
        if( msgrcv( msgQId, &msg, sizeof(message), POLECENIE_2_MSG_ID, IPC_NOWAIT ) != -1 || msgrcv( msgQId, &msg, sizeof(message), MESSAGES_WORKERS, IPC_NOWAIT ) != -1 ){
            say("got message");
            break;
        }
        sleep(5);
        if( getMaterials( semId, storage ) ){
            work();
        }
        else
            say("No materials!");
    }

    say("job done");
    return 0;
}

// 1 -> success, 0 -> failure
int getMaterials( int semId, storageSegment *storage ){
    if( semLower( semId, SEM_WORKERS ) || semLower( semId, SEM_DELIVERY ) ){
        warning("No storage detected - closing");
        STORAGE_EXISTS = 0;
        return 0;
    }
    // check for empty
    for( int i = 0; i < 3; i++ ){
        char value = storage[i].start[*storage[i].read];
        if( !value ){ //empty
            semRaise(semId, SEM_DELIVERY);
            semRaise(semId, SEM_WORKERS);
            return 0;
        }
    }
    // take what needed
    for( int i = 0; i < 3; i++ ){
        memset( storage[i].start + *storage[i].read, 0, storage[i].elSize );
        *storage[i].read += storage[i].elSize;
        if( storage[i].start + *storage[i].read >= storage[i].end )
            *storage[i].read = 0;
    }

    #if DEBUG
        for( int i = 0; i < 3; i++ ){
            printf( "storage int[%d]: %p - %p\n", i, storage[i].start, storage[i].end );
            printf( "access     [%d]: r -%p, w - %p\n", i, *storage[i].read, *storage[i].write );
        }
    #endif
    semRaise(semId, SEM_DELIVERY);
    semRaise(semId, SEM_WORKERS);

    return 1;
}

int work(){
    say("work work");
    return 0;
}