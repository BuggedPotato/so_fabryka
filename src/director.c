#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<errno.h>
#include "../include/constants.h"
#include "../include/utils.h"
#include "../include/types.h"
pid_t PID;
int msgQId;

int deleteMessageQueue( int msgQId );
int sendMessage( int msgQId, message *msg );

void storageCloseHandler( int sig );

int main(int argc, char *argv[]){
    PID = getpid();
    
    signal(SIGUSR1, storageCloseHandler);

    key_t msgQKey = getKey( MSGQ_KEY_STRING, MSGQ_KEY_CHAR );
    msgQId = getMessageQueue( msgQKey, 0340 );

    message msg; 
    char c, foo;
    int count = 1;
    printf("\e[2J");
    say("Waiting for input");
    printf("\e[H\e[KWaiting for input:");
    while( 1 ){
        c = fgetc(stdin);
        while ((foo = getchar()) != '\n' && foo != EOF);
        count = 1;
        switch (c)
        {
            case '1':
                msg.type = POLECENIE_1_MSG_ID;
                break;
            case '2':
                msg.type = POLECENIE_2_MSG_ID;
                count = WORKERS;
                break;
            case '3':
                msg.type = POLECENIE_3_MSG_ID;
                count = WORKERS + 1;
                break;
            case '4':
                msg.type = POLECENIE_4_MSG_ID;
                count = WORKERS + 1;
                break;
            case '5':
                warning("Quitting");
                printf("Quitting\n");
                continue;
                break;
            default:
                printf("\e[H\e[KInvalid input - options: 1, 2, 3, 4, 5");
                continue;
                break;
        }
        for( int i = 0; i < count; i++ )
            sendMessage( msgQId, &msg );
        say("Message sent");
        printf("\e[2J\e[H\e[KMessage sent\n");
    }
    deleteMessageQueue(msgQId);

    return 0;
}

int sendMessage( int id, message *msg ){
    if( msgsnd( id, (void *)msg, sizeof(message), 0 ) == -1 ){
        perror("error sending message");
        exit(errno);
    }
    return 0;
}

int deleteMessageQueue( int id ){
    if( msgctl( id, IPC_RMID, NULL ) == -1 ){
        perror("error deleting message queue");
        exit(errno);
    }
    say("Message queue successfully deleted");
    return 0;
}

void storageCloseHandler( int sig ){
    say("Storage closing confirmed");
    printf("Storage closing confirmed\n");
    deleteMessageQueue(msgQId);
    say("Shutting down...");
    printf("Shutting down...\n");
    exit(EXIT_SUCCESS);
}