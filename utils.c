#define _GNU_SOURCE
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<sys/msg.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>

extern char *program_invocation_short_name;

int fileExists(char *fileName){
    struct stat buffer;
    return (stat(fileName, &buffer) == 0);
}

key_t getKey( char* path, int id ){
    key_t key = ftok( path, id );
    // if( key == -1 ){
    //     perror("Key error");
    //     return -1;
    // }
    return key;
}

void say( char *text ){
    printf("[%s %d] %s\n", program_invocation_short_name, getpid(), text);
}

int getSemaphores( key_t key, int count, int perms ){
    int semId;
    if( (semId = semget(key, count, IPC_CREAT|perms)) == -1 ){
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