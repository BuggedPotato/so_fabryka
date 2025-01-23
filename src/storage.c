#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
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

int createStorage(key_t key, int size);
int storageSetup(int shmId, char **shmAddr, int size);
int getOptimalSegmentSize(int M);
int deleteStorage( int shmId, int semId, char *shmAddr, int size );
int loadStorageFile( char *fileName, char *dest, char *end );
int saveStorageFile( char *fileName, char *src, char *end );

int semaphoresSetup( int semId );
int deleteSemaphores( int semId );

pid_t PID;
int SAVE_TO_FILE = 0;

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
    int semId = getSemaphores( semKey, 3, 0600 );
    semaphoresSetup( semId );
    
    say("Sending continue signal");
    printf("%d\n", getppid());
    if( kill( getppid(), SIGCONT ) == -1)
        error("error sending continue");

    key_t msgQKey = getKey( MSGQ_KEY_STRING, MSGQ_KEY_CHAR );
    int msgQId = getMessageQueue( msgQKey, 0700 );

    message msg;
    say("Entering standby mode - awaiting messages...");
    if( msgrcv( msgQId, (void *)&msg, sizeof(message), MESSAGES_STORAGE, 0 ) == -1 ){
        perror("error receiving a message");
        exit(EXIT_FAILURE);
    }
    // here ppid is director
    say( "Got a message!" );
    if( msg.type == POLECENIE_3_MSG_ID )
        SAVE_TO_FILE = 1;

    deleteStorage( shmId, semId, (void *)shmAddr, STORAGE_TOTAL_SIZE );
    deleteSemaphores(semId);
    kill( getppid(), SIGUSR1 );
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

int deleteStorage( int shmId, int semId, char *shmAddr, int size ){
    say("waiting for sems");
    if( semLower( semId, SEM_STORAGE ) == -1 ){
        error("No semaphores found");
        exit(EXIT_FAILURE);
    }
    say("storage past sems");
    // say("sem deliver");
    // semLower( semId, SEM_STORAGE );
    // say("sem workers");
    saveStorageFile( STORAGE_FILENAME, shmAddr, shmAddr + size );
    if( shmdt( (void *)shmAddr ) == -1 ){
        perror("error detaching shared memory");
        exit(errno);
    }
    if( shmctl( shmId, IPC_RMID, NULL ) == -1 ){
        perror( "error deleting shared memory" );
        exit( errno );
    }
    say("Successfully deleted storage");
    //no raising because theyre deleted right after
    // semRaise(semId, SEM_DELIVERY);
    // semRaise(semId, SEM_STORAGE);
    //semRaise(semId, SEM_QUEUE);
    return 0;
}

int semaphoresSetup( int semId ){
    if( semctl( semId, SEM_STORAGE, SETVAL, 1 ) == -1 || semctl( semId, SEM_DELIVERY, SETVAL, 1 ) == -1 || semctl(semId, SEM_QUEUE, SETVAL, 1) == -1 ){
        perror("cannot set semaphores init value");
        exit(errno);
    }
    success("Semaphores setup compete");
    return 0;
}

int deleteSemaphores( int semId ){
    // fprintf(stderr, "%d\n", semId);
    if( semctl( semId, 0, IPC_RMID ) == -1 ){
        perror("cannot delete semaphore set");
        exit(errno);
    }
    say("Successfully deleted semaphore set");    
    return 0;
}

int loadStorageFile( char *fileName, char *dest, char *end ){
    say("Loading storage file");
    FILE* file = NULL;
    if( (file = fopen( fileName, "r" )) == NULL ){
        warning("No saved storage file found! Starting clean");
        printf("\e[HNo saved storage file found! Starting clean");
        memset( dest, 0, end - dest );
        return -1;
    }
    char buffer[128];
    memset(dest, 0, end - dest);
    int res = 0;
    int shift = 0;
    while( (res = fread( buffer, sizeof(char), 256, file )) != 0 ){
        if( dest + shift + res > end ){
            error("Input file too long wiping storage");
            fclose(file);
            memset( dest, 0, end - dest );
            return 1;
        }
        memcpy( dest + shift, buffer, res * sizeof(char) );
        for( int i = 0; i < res; i++ ){
            fprintf( stderr, "%02X ", *(dest + shift + i) );
            if( (i+1)%16 == 0 )
                fprintf(stderr, "\n");
        }
        shift += res;
    }
    fclose( file );
    say("File loaded successfully");
    printf("\e[HFile loaded successfully");
    return 0;
}

int saveStorageFile( char *fileName, char *src, char *end ){
    FILE* file = NULL;
    if( (file = fopen( fileName, "w" )) == NULL ){
        error("Could not open save file! Aborting");
        exit(errno);
    }
    if( !SAVE_TO_FILE )
        return 0;
    say("Saving storage file");

    if( fwrite( src, sizeof(char), end-src, file ) != end - src ){
        error("Invalid number of elements written");
    }
    fclose( file );
    say("File saved successfully");
    return 0;
}