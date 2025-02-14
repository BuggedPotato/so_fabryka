#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<time.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/sem.h>
#include<sys/msg.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<errno.h>
#include "../include/constants.h"
#include "../include/types.h"
#include "../include/utils.h"
#include "../include/semaphores.h"

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
    int semId = getSemaphores( semKey, 3, 0600 );
    say("Worker successfully attached semaphores");

    key_t msgQKey = getKey( MSGQ_KEY_STRING, MSGQ_KEY_CHAR );
    int msgQId = getMessageQueue( msgQKey, 0700 );
    say("Worker successfully attached message queue");

    message msg;
    srand(PID);
    int res = 0;
    while(STORAGE_EXISTS){
        // TODO?
        if( msgrcv( msgQId, &msg, sizeof(message), POLECENIE_2_MSG_ID, IPC_NOWAIT ) != -1 || msgrcv( msgQId, &msg, sizeof(message), MESSAGES_WORKERS, IPC_NOWAIT ) != -1 ){
            say("got message");
            break;
        }
        #if SPEED != NO_SLEEP
            #if SPEED == SLOW
                sleep(1+rand() % 15);
            #elif SPEED == FAST
                usleep(250+rand() % 1500);
            #endif
        #endif
        if( (res = getMaterials( semId, storage )) == 1 ){
            work();
            #if VERBOSE
                success("work work");
            #endif
        }
        else if( res == 0 ){
            #if VERBOSE
                say("No materials!");
            #endif
        }
    }

    msg.type = WORKER_CLOSING_MSG_ID;
    if( msgsnd( msgQId, (void *)&msg, MSG_TEXT_SIZE, 0 ) == -1 ){
        perror("error sending message");
        error("error sending message");
        exit(errno);
    }
    success("job done");
    return 0;
}

/*
* Function Name:	getMaterials
*
* Function:			attempts to take materials from storage
*
* Arguments:		semId - semaphore set id,
                    storage - storage segment struct array
*
* Return:			1->success, 0->at least one segment empty, -1->semaphore error 
*/
int getMaterials( int semId, storageSegment *storage ){
    if(semLower(semId, SEM_QUEUE) == -1){
        warning("No queue detected - closing");
        STORAGE_EXISTS = 0;
        return -1;
    }
    if( semLower( semId, SEM_STORAGE ) ){
        warning("No storage detected - closing");
        STORAGE_EXISTS = 0;
        return -1;
    }
    // check for empty
    for( int i = 0; i < 3; i++ ){
        char value = storage[i].start[*storage[i].read];
        if( !value ){ //empty
            semRaise(semId, SEM_STORAGE);
            semRaise(semId, SEM_QUEUE);
            return 0;
        }
    }
    // take what needed
    int position[3] = {0, 0, 0};
    for( int i = 0; i < 3; i++ ){
        position[i] = *storage[i].read;
        memset( (void *)(storage[i].start + *storage[i].read), 0, storage[i].elSize );
        *storage[i].read += storage[i].elSize;
        if( storage[i].start + *storage[i].read >= storage[i].end )
            *storage[i].read = 0;
    }

    #if DEBUG
        for( int i = 0; i < 3; i++ ){
            printf( "storage int[%d]: %p - %p\n", i, storage[i].start, storage[i].end );
            printf( "access     [%d]: r -%d, w - %d\n", i, *storage[i].read, *storage[i].write );
        }
    #endif
    semRaise(semId, SEM_STORAGE);
    semRaise(semId, SEM_QUEUE);

    return 1;
}


// dummy function to simulate working 
int work(){
    return 0;
}