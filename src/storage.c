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

int drawStorageDiff( char* last, char* curr, int size);

pid_t PID;
int SAVE_TO_FILE = 0;
time_t LAST_DRAW = 0;

int main(int argc, char *argv[]){
    PID = getpid();

    key_t shmKey = getKey( STORAGE_KEY_STR, STORAGE_KEY_CHAR );
    char *shmAddr = NULL;
    int shmId = createStorage( shmKey, STORAGE_TOTAL_SIZE );
    storageSegment segments[3];
    if( storageSetup(shmId, &shmAddr, STORAGE_TOTAL_SIZE) ){
        perror( "error building storage structure" );
        exit(EXIT_FAILURE);
    }

    key_t semKey = getKey( SEM_KEY_STR, SEM_KEY_CHAR );
    int semId = getSemaphores( semKey, 3, 0600 );
    semaphoresSetup( semId );
    
    // signal to manager (parent) to continue with other processes
    say("Sending continue signal");
    printf("%d\n", getppid());
    if( kill( getppid(), SIGCONT ) == -1)
        error("error sending continue");

    key_t msgQKey = getKey( MSGQ_KEY_STRING, MSGQ_KEY_CHAR );
    int msgQId = getMessageQueue( msgQKey, 0700 );

    message msg;
    // shm snapshot for drawing
    char shmCopy[STORAGE_TOTAL_SIZE];
    semLower(semId, SEM_STORAGE);
    memcpy( shmCopy, shmAddr, STORAGE_TOTAL_SIZE );
    semRaise(semId, SEM_STORAGE);
    say("Awaiting messages...");
    while( 1 ){
        int res = msgrcv( msgQId, (void *)&msg, MSG_TEXT_SIZE, MESSAGES_STORAGE, IPC_NOWAIT );
        if( res == -1 ){
            if( errno != ENOMSG ){
                perror("error receiving a message");
                exit(EXIT_FAILURE);
            }
        }
        else{
            break;
        }

        // draw storage to terminal if enough time has passed
        time_t n;
        if( time(&n) - LAST_DRAW >= 1 || LAST_DRAW == 0 ){
            LAST_DRAW = n;
            semLower(semId, SEM_STORAGE);
            drawStorageDiff( shmCopy, shmAddr, STORAGE_TOTAL_SIZE );
            memcpy( shmCopy, shmAddr, STORAGE_TOTAL_SIZE );
            semRaise(semId, SEM_STORAGE);
        }
    }
    
    say( "Got a message!" );
    if( msg.type == POLECENIE_3_MSG_ID )
        SAVE_TO_FILE = 1;

    deleteStorage( shmId, semId, (void *)shmAddr, STORAGE_TOTAL_SIZE );
    deleteSemaphores(semId);
    msg.type = STORAGE_CLOSING_MSG_ID;
    if( msgsnd( msgQId, (void *)&msg, MSG_TEXT_SIZE, 0 ) == -1 ){
        perror("error sending message");
        error("error sending message");
        exit(errno);
    }
    say("Shutting down");

    return 0;
}
/*
* Function Name:	createStorage
*
* Function:			allocate shared memory with shmget
*
* Arguments:		key - shm key (from getKey),
                    size - shm size in bytes
*
* Return:			id of created shared memory, exit on error
*/
int createStorage(key_t key, int size){
    
    int shmId = shmget( key, size, IPC_CREAT|0700 );
    if( shmId == -1 ){
        perror("Shared memory allocation error");
        exit( errno );
    }
    return shmId;
}

/*
* Function Name:	storageSetup
*
* Function:			attach shared memory and load data from storage file
*
* Arguments:		shmId - shared memory id,
                    shmAddr - address of pointer to shared memory (assigns to original pointer),
                    size - shm size in bytes
*
* Return:			0->success, exit error -> failure/error
*/
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

/*
* Function Name:	deleteStorage
*
* Function:			detach and remove shared memory
*
* Arguments:		shmId - shared memory id,
            		semId - semaphore set id,
                    shmAddr - pointer to shared memory,
                    size - shm size in bytes
*
* Return:			0->success, exit error -> failure/error
*/
int deleteStorage( int shmId, int semId, char *shmAddr, int size ){
    say("waiting for semaphores");
    if( semLower( semId, SEM_STORAGE ) == -1 ){
        error("No semaphores found");
        exit(EXIT_FAILURE);
    }
    say("storage past sems");

    saveStorageFile( STORAGE_FILENAME, shmAddr, shmAddr + size );
    if( shmdt( (void *)shmAddr ) == -1 ){
        perror("error detaching shared memory");
        exit(errno);
    }
    if( shmctl( shmId, IPC_RMID, NULL ) == -1 ){
        perror( "error deleting shared memory" );
        exit( errno );
    }
    success("Successfully deleted storage");
    //no raising because theyre deleted right after
    return 0;
}

/*
* Function Name:	semaphoresSetup
*
* Function:			set semaphores intitial values
*
* Arguments:		semId - semaphore set id
*
* Return:			0->success, exit error -> failure/error
*/
int semaphoresSetup( int semId ){
    if( semctl( semId, SEM_STORAGE, SETVAL, 1 ) == -1 || semctl( semId, SEM_DELIVERY, SETVAL, 1 ) == -1 || semctl(semId, SEM_QUEUE, SETVAL, 1) == -1 ){
        perror("cannot set semaphores init value");
        exit(errno);
    }
    success("Semaphores setup compete");
    return 0;
}


/*
* Function Name:	deleteSemaphores
*
* Function:			delete semaphore set
*
* Arguments:		semId - semaphore set id
*
* Return:			0->success, exit error -> failure/error
*/
int deleteSemaphores( int semId ){
    if( semctl( semId, 0, IPC_RMID ) == -1 ){
        perror("cannot delete semaphore set");
        exit(errno);
    }
    say("Successfully deleted semaphore set");    
    return 0;
}

/*
* Function Name:	loadStorageFile
*
* Function:			load data from file into storage (shared memory)
*
* Arguments:		fileName - input file name,
                    dest - destination,
                    end - end of available addresses (final address + 1)
*
* Return:			0->success, 1->file too long, -1->no file found
*/
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

/*
* Function Name:	saveStorageFile
*
* Function:			sava data from storage (shared memory) to file
*
* Arguments:		fileName - input file name,
                    src - data source,
                    end - end of available addresses (final address + 1)
*
* Return:			0->success, exit on error
*/
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

/*
* Function Name:	drawStorageDiff
*
* Function:			draw a visualisation of storage (shared memory) in terminal,
                    marks changes between last draw with colours:
                    green = added, red = removed
*
* Arguments:		last - last drawn shared memory copy,
                    curr - address of current shared memory state,
                    size - size of both in bytes
*
* Return:			0->not drawn, exit on error
*/
int drawStorageDiff( char* last, char* curr, int size){
    printf("\e[2;1H");
    char t[10];
    getTime(t);
    printf("\e[0JLast update: %s\n", t);
    // colours for seperating elements
    char colours[3][6] = { "\e[36m", "\e[35m", "\e[37m" };
    char def[] = "\e[39m"; // default text colour
    int c = 0;

    int line = 0, highlight = 0;
    int indexes = size - 6 * sizeof(int); // index of read/write fields in shm
    int elSizes[3] = {SIZE_X, SIZE_Y, SIZE_Z};
    int el = 0;
    int segment = 0;
    for( int j = 0; j < 3; j++ ){
        printf("read: %d, write: %d\n", curr[indexes+2*j], curr[indexes+2*j+1]);
    }
    for( int i = 0; i < indexes; i++ ){
        if( i % elSizes[segment] == 0 ){
            printf(colours[c]);
            c++;
            if( c >= 3 ) c = 0;
        }
        el++;

        if( !last[i] && curr[i] ){
            // green
            printf("\e[42m");
        }
        else if( last[i] && !curr[i] ){
            // red
            printf("\e[41m");
        }
        printf("%02X ", curr[i]);
        printf("\e[49m");

        line++;
        if( line % 4 == 0 )
            printf(" ");
        if( line >= 8 ){
            printf("\n");
            line = 0;
        }
        // seperates segments
        if( el == elSizes[segment] * STORAGE_COUNT ){
            printf("\n");
            segment++;
            el = 0;
        }
    }
    printf(def);
    return 1;
}