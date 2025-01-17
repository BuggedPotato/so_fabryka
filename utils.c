#define _GNU_SOURCE
#include<sys/stat.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<sys/shm.h>
#include<sys/msg.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>

#include "constants.h"
#include "types.h"

extern char *program_invocation_short_name;

int fileExists(char *fileName){
    struct stat buffer;
    return (stat(fileName, &buffer) == 0);
}

key_t getKey( char* path, int id ){
    key_t key = ftok( path, id );
    return key;
}

void say( char *text ){
    printf("[%s %d] %s\n", program_invocation_short_name, getpid(), text);
}

void warning( char *text ){
    printf("[%s %d]\e[33m[WARNING]\e[0m %s\n", program_invocation_short_name, getpid(), text);
}

void error( char *text ){
    printf("[%s %d]\e[31m[ERROR]\e[0m %s\n", program_invocation_short_name, getpid(), text);
}

int getStorage(key_t key){
    int shmId = shmget( key, 0, IPC_CREAT|0600 );
    if( shmId == -1 ){
        error("");
        perror("Shared memory get error");
        exit( errno );
    }
    return shmId;
}

char* attachStorage( int id ){
    void *tmpPtr;
    tmpPtr = shmat( id, NULL, 0 );
    if( tmpPtr == (void *)-1 ){
        error("");
        perror("Error attaching shared memory");
        exit(errno);
    }
    return (char *)tmpPtr;
}

int getSemaphores( key_t key, int count, int perms ){
    int semId;
    if( (semId = semget(key, count, IPC_CREAT|perms)) == -1 ){
        error("");
        perror("error creating semaphores");
        exit(errno);
    }
    return semId;
}

int getMessageQueue( key_t key, int perms ){
    int msgQId = msgget( key, IPC_CREAT|perms );
    if( msgQId == -1 ){
        perror("error getting message queue");
        exit(errno);
    }
    return msgQId;
}

int processExists( pid_t pid ){
    return kill( pid, 0 ) == 0;
}

int getStorageSegments( char* shmAddr, storageSegment *storageSegments ){
    int baseSize = STORAGE_COUNT;
    storageSegments[0].start = shmAddr;
    storageSegments[0].end = storageSegments[0].start + baseSize;
    storageSegments[0].elSize = SIZE_X;
    int sizes[3] = {SIZE_X, SIZE_Y, SIZE_Z};
    for( int i = 1; i < 3; i++ ){
        storageSegments[i].start = storageSegments[i-1].end;
        storageSegments[i].end = storageSegments[i].start + sizes[i] * baseSize;
        storageSegments[i].elSize = sizes[i];
    }

    storageSegments[0].read = (int *)storageSegments[2].end;
    storageSegments[0].write = storageSegments[0].read + 1;
    *storageSegments[0].read = 0;
    *storageSegments[0].write = 0;
    for( int i = 1; i < 3; i++ ){
        storageSegments[i].read = storageSegments[i-1].write + 1;
        storageSegments[i].write = storageSegments[i].read + 1;
        *storageSegments[i].read = 0;
        *storageSegments[i].write = 0;
    }
    return 0;
}