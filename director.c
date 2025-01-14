#include<stdlib.h>
#include<sys/types.h>
#include<sys./ipc.h>
#include<sys/shm.h>
#include<errno.h>
#include "constants.h"
#include "utils.h"

int main(int argc, char *argv[]){

    int storageSize = DEFAULT_STORAGE_SIZE;
    if( argc > 1 )
        storageSize = atoi(argv[1]);
    int shmId = getStorage(storageSize);

    return 0;
}

/*
    creates shm of 'size' elements + delivery and recieve value points
    size = M
*/
int getStorage(int size){
    if( !fileExists(STORAGE_FILENAME) ){
        FILE *ptr;
        ptr = fopen(STORAGE_FILENAME, "w");
        if( ptr == NULL ){
            perror("Storage file missing could not create a new file");
            exit -2;
        }
        fclose(ptr);
    }
    
    key_t key = ftok( "./storage", 'S' );
    if( key == -1 ){
        perror("Key error");
        return -1;
    }
    int sizeBytes = size + 6 * sizeof(int);
    int shmId = shmget( key, size, IPC_CREAT|200 );
    if( shmId == -1 ){
        perror("Shared memory allocation error");
        return errno;
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