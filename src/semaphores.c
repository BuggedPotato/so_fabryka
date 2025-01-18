#include<sys/ipc.h>
#include<sys/sem.h>
#include<stdlib.h>
#include<errno.h>
#include<stdio.h>

int semLower( int id, int semNum ){
    struct sembuf op;
    op.sem_num = semNum;
    op.sem_op = -1;
    op.sem_flg = 0;
    if( semop( id, &op, 1 ) == -1 ){
        // perror("could not lower semaphore");
        // exit(EXIT_FAILURE);
        return 1;
    }
    return 0;
}

int semRaise( int id, int semNum ){
    struct sembuf op;
    op.sem_num = semNum;
    op.sem_op = 1;
    op.sem_flg = 0;
    if( semop( id, &op, 1 ) == -1 ){
        // perror("could not raise semaphore");
        // exit(EXIT_FAILURE);
        return 1;
    }
    return 0;
}