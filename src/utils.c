#define _GNU_SOURCE
#include<sys/stat.h>
#include<signal.h>
#include<time.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<sys/shm.h>
#include<sys/msg.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>

#include "../include/constants.h"
#include "../include/types.h"

// time_t LAST_DRAW = 0;

extern char *program_invocation_short_name;
void getTime( char *dest );

key_t getKey( char* path, int id ){
    key_t res = ftok(path, id);
    if( res == -1 ){
        perror("error getting key");
        exit(errno);
    }
    return res;
}

/*
* Function Name:	logPrint
*
* Function:			printsd formatted message with time, pid 
                    and special label to stderr 
*
* Arguments:		label - extra label to add to message (e.g. [WARNING])
                    text - message string
*/
void logPrint( char* label, char* text ){
    char t[9];
    getTime( t );
    fprintf(stderr, "[%s][%s %d]%s %s\n", t, program_invocation_short_name, getpid(), label, text);
}
// refer to logPrint
void say( char *text ){
    logPrint("", text);
}
// refer to logPrint
void warning( char *text ){
    logPrint("\e[33m[WARNING]\e[0m", text);
}
// refer to logPrint
void error( char *text ){
    logPrint("\e[31m[ERROR]\e[0m", text);
}
// refer to logPrint
void success( char *text ){
    logPrint("[32m[OK]\e[0m", text);
}

/*
* Function Name:	getTime
* Function:			puts formatted time (hours:min:sec) string into dest
* Arguments:		dest - destination for time string
*/
void getTime( char *dest ){
    time_t tmp;
    time(&tmp);
    struct tm *t = localtime(&tmp);
    sprintf( dest, "%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);
}

/*
* Function Name:	getStorage
* Function:			get shared memory id for key
* Arguments:		key - shm key
* Return:			obtained id on success, exit on failure/error
*/
int getStorage(key_t key){
    int shmId = shmget( key, 0, IPC_CREAT|0600 );
    if( shmId == -1 ){
        error("");
        perror("Shared memory get error");
        exit( errno );
    }
    return shmId;
}

/*
* Function Name:	attachStorage
* Function:			attaches shared memory
* Arguments:		id - shm id
* Return:			obtained shm address, exit on failure/error
*/
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

/*
* Function Name:	getSemaphores
* Function:			returns semaphore set id for key with given permissions,
                    can create a new set or attach to existing
* Arguments:		key - semaphore set key,
                    count - semaphores count,
                    perms - permissions
* Return:			obtained semaphore set id, exit on failure/error
*/
int getSemaphores( key_t key, int count, int perms ){
    int semId;
    if( (semId = semget(key, count, IPC_CREAT|perms)) == -1 ){
        error("");
        perror("error creating semaphores");
        exit(errno);
    }
    return semId;
}

/*
* Function Name:	getMessageQueue
* Function:			returns message queue id for key with given permissions
* Arguments:		key - semaphore set key,
                    perms - permissions
* Return:			obtained message queue id, exit on failure/error
*/
int getMessageQueue( key_t key, int perms ){
    int msgQId = msgget( key, IPC_CREAT|perms );
    if( msgQId == -1 ){
        perror("error getting message queue");
        exit(errno);
    }
    return msgQId;
}

/*
* Function Name:	getStorageSegments
* Function:			structs to represent storage logic design
* Arguments:		shmAddr - shared memory address,
                    storageSegments - array of structs to be set
* Return:			0->success
*/
int getStorageSegments( char* shmAddr, storageSegment *storageSegments ){
    int baseSize = STORAGE_COUNT;
    storageSegments[0].start = shmAddr;
    storageSegments[0].end = storageSegments[0].start + baseSize * SIZE_X;
    storageSegments[0].elSize = SIZE_X;
    int sizes[3] = {SIZE_X, SIZE_Y, SIZE_Z};
    for( int i = 1; i < 3; i++ ){
        storageSegments[i].start = storageSegments[i-1].end;
        storageSegments[i].end = storageSegments[i].start + sizes[i] * baseSize;
        storageSegments[i].elSize = sizes[i];
    }

    storageSegments[0].read = (int *)storageSegments[2].end;
    storageSegments[0].write = storageSegments[0].read + 1;
    for( int i = 1; i < 3; i++ ){
        storageSegments[i].read = storageSegments[i-1].write + 1;
        storageSegments[i].write = storageSegments[i].read + 1;
    }
    return 0;
}

/*
  /// NOT IN USE ///
* Function Name:	drawStorage
* Function:			draws storage visualisation to terminal with 
                    coloured highlighting
* Arguments:		storage - array of structs to read from,
                    position - array with changed index for each storage segment,
                    operation - 1->added, 0->removed (colouring)
*/
void drawStorage( storageSegment *storage, int *position, int operation ){

    printf("\e[2;1H");
    char t[10];
    getTime(t);
    printf("\e[0JLast update: %s\n", t);
    char colours[3][6] = { "\e[36m", "\e[35m", "\e[37m" };
    char def[] = "\e[39m";
    int c = 0;
    for( int i = 0; i < 3; i++ ){
        int line = 0;
        printf("\n\e[2Kr: %d, w: %d %s %d\n", *storage[i].read, *storage[i].write, program_invocation_short_name, position[i]);
        for( char *b = storage[i].start; b < storage[i].end; ){
            printf(colours[c]);
            int highlight = 0;
            if( position[i] >= 0 && (b - storage[i].start) == position[i] ){
                highlight = 1;
                if( operation )
                    printf("\e[42m");
                else
                    printf("\e[41m");
            }
            for( int j = 0; j < storage[i].elSize; j++, b++ ){
                printf("%02X ", *b);
                line++;
                if( line % 4 == 0 )
                    printf(" ");
                if( line >= 8 ){
                    printf("\n");
                    line = 0;
                }
            }
            if( highlight ){
                printf("\e[49m");
            }
            printf(def);
            c++;
            if( c >= 3 ) c = 0;
        }
    }
}
