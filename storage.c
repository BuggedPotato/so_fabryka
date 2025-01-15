#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<errno.h>
#include "constants.h"
#include "utils.h"

int main(int argc, char *argv[]){

    int shmKey = 0;
    if( argc < 2 ){
        perror("not enough arguments");
        exit(-1);
    }
    shmKey = atoi(argv[1]);
    if( shmKey <= 0 ){
        perror( "Invalid shm key" );
        exit(-1);
    }

    int shmId = getStorage( shmKey, STORAGE_SIZE );
    // TODO message queue

    return 0;
}

/*
    creates shm of 'size' elements + delivery and recieve value points
    size = M
*/
int getStorage(key_t key, int size){
    
    int sizeBytes = size + 6 * sizeof(int);
    int shmId = shmget( key, size, IPC_CREAT|200 );
    if( shmId == -1 ){
        perror("Shared memory allocation error");
        exit( errno );
    }
    // TODO saved file write
    return shmId;
}

/*
    M elements total
*/
int getOptimalSegmentSize(int M){
    return M / (SIZE_X + SIZE_Y + SIZE_Z);
}