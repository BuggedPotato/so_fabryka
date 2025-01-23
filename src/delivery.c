#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<string.h>
#include<time.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<errno.h>
#include "../include/constants.h"
#include "../include/utils.h"
#include "../include/types.h"
#include "../include/semaphores.h"
pid_t PID;
int STORAGE_EXISTS = 1;

int deliver( int semId, storageSegment *storage, int el );

int main(int argc, char *argv[]){

    if( argc < 2 ){
        perror("Not enough arguments");
        exit(EXIT_FAILURE);
    }
    int el = atoi(argv[1]);
    if( el <= 0 || el > 3 ){
        errno = EINVAL;
        perror( "Invalid element" );
        exit(errno);
    }
    el--;

    key_t shmKey = getKey( STORAGE_KEY_STR, STORAGE_KEY_CHAR );
    int shmId = getStorage( shmKey );
    storageSegment storage[3];
    char *shmAddr = attachStorage( shmId );
    getStorageSegments( shmAddr, storage );

    key_t semKey = getKey( SEM_KEY_STR, SEM_KEY_CHAR );
    int semId = getSemaphores( semKey, 3, 0600 );

    srand(PID);
    int res = 0;
    while(STORAGE_EXISTS){
        #if SPEED != NO_SLEEP
            #if SPEED == SLOW
                sleep(1+rand() % 10);
            #elif SPEED == FAST
                usleep(200+rand() % 1000);
            #endif
        #endif
        res = deliver( semId, storage, el );
        #if VERBOSE
            if( res == 0 ){
                say( "Storage full - skipping" );
            }
            else if( res == 1 ){
                success( "Delivery done!" );
            }
        #endif
    }

    say("Quitting...");
    return 0;
}

// 1 -> success, 0 -> failure
int deliver( int semId, storageSegment *fullStorage, int el ){
    if( semLower(semId, SEM_QUEUE) == -1 ){
        warning("No queue detected - closing");
        STORAGE_EXISTS = 0;
        return -1;
    }
    if( semLower( semId, SEM_STORAGE ) ){
        warning("No storage detected - closing");
        STORAGE_EXISTS = 0;
        return -1;
    }
    storageSegment *storage = &fullStorage[el];
    // check for full
    char value = storage->start[*(storage->write)];
    if( value ){ //full
        // semRaise(semId, SEM_DELIVERY);
        semRaise(semId, SEM_STORAGE);
        semRaise(semId, SEM_QUEUE);
        return 0;
    }

    #if DEBUG
        printf("size: %d\n", storage->elSize);
        printf( "delivery data: %p - %p\n", storage->start, storage->end );
        printf( "access     : r - %d, w - %d\n", *storage->read, *storage->write );
    #endif
    // deliver
    // for( int i = 0; i < 3; i++ ){
        int position[3] = {-1, -1, -1};
        position[el] = *(storage->write);
        memset( (void *)(storage->start + *(storage->write)), 1, storage->elSize );
        *(storage->write) += storage->elSize;
        if( storage->start + *(storage->write) >= storage->end )
            *(storage->write) = 0;
    // }

    #if DEBUG
        printf( "delivery data: %p - %p\n", storage->start, storage->end );
        printf( "access     : r - %d, w - %d\n", *storage->read, *storage->write );
    #endif
    #if VERBOSE
    #endif
        drawStorage( fullStorage, position );
    // semRaise(semId, SEM_DELIVERY);
    semRaise(semId, SEM_STORAGE);
    semRaise(semId, SEM_QUEUE);

    return 1;
}


