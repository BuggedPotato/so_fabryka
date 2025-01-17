#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<errno.h>
#include "constants.h"
#include "utils.h"
#include "types.h"
pid_t PID;

int deleteMessageQueue( int msgQId );

int main(int argc, char *argv[]){
    PID = getpid();

    int storageSize = STORAGE_SIZE;
    if( argc > 1 )
        storageSize = atoi(argv[1]);
    if( storageSize <= 0 ){
        perror( "Invalid storage size" );
        errno = EINVAL;
        exit(errno);
    }

    /* create all the child processes here */
    pid_t storagePID;
    if( (storagePID = fork()) == -1 ){
        perror("storage fork error");
        exit(errno);
    }
    else if( storagePID == 0 ){
        if( execl( "./storage", "storage", NULL )  == -1){
            perror("error running storage process");
            exit(errno);
        }
    }

    pid_t workersPID[WORKERS];
    for( int i = 0; i < WORKERS; i++ ){
        if( (workersPID[i] = fork()) == -1 ){
            perror("worker fork error");
            exit(errno);
        }
        else if( workersPID[i] == 0 ){
            if( execl( "./worker", "worker", NULL )  == -1){
                perror("error running worker process");
                exit(errno);
            }
        }
    }


    key_t msgQKey = getKey( MSGQ_KEY_STRING, MSGQ_KEY_CHAR );
    int msgQId = getMessageQueue( msgQKey, 0700 );
    message msg; 
    // msg.type = POLECENIE_1_MSG_ID;
    // sleep(2);
    // say("Sending message Polecenie_1");
    // if( msgsnd( msgQId, (void *)&msg, MSG_TEXT_SIZE, 0 ) == -1 ){
    //     perror("error sending message");
    //     exit(errno);
    // }
    // say("Message sent!");

    sleep(10);
    msg.type = POLECENIE_2_MSG_ID;
    say("Sending messages Polecenie_2");
    for( int i = 0; i < WORKERS; i++ ){
        if( msgsnd( msgQId, (void *)&msg, MSG_TEXT_SIZE, 0 ) == -1 ){
            perror("error sending message");
            exit(errno);
        }
    }
    say("Messages sent!");

    int status;
    // waitpid(storagePID, &status, 0);
    // printf( "%d\n", WEXITSTATUS(status) );
    // say("Storage closed");
    for( int i = 0; i < WORKERS + 1; i++ ){
        int res = wait( &status );
        if( WIFEXITED(status) )
            printf( "%d - %d\n", res, WEXITSTATUS(status) );
        else printf("%d\n", errno);
    }
    say("Children closed");
    deleteMessageQueue(msgQId);

    say("Shutting down...");

    return 0;
}

int deleteMessageQueue( int msgQId ){
    if( msgctl( msgQId, IPC_RMID, NULL ) == -1 ){
        perror("error deleting message queue");
        exit(errno);
    }
    say("Message queue successfully deleted");
    return 0;
}
