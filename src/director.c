#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/select.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<errno.h>
#include "../include/constants.h"
#include "../include/utils.h"
#include "../include/types.h"
pid_t PID;

int STORAGE_RUNNNING = 1, FACTORY_RUNNING = WORKERS;

int deleteMessageQueue( int msgQId );
int sendMessage( int msgQId, message *msg );

void handle(int sig){
    printf("signal: %d\n", sig);
}

int main(int argc, char *argv[]){
    PID = getpid();
    say("Started");
    struct sigaction sa;
    sa.sa_handler = SIG_DFL;
    sa.sa_flags = SA_NOCLDWAIT;
    sigaction( SIGCHLD, &sa, NULL );

    key_t msgQKey = getKey( MSGQ_KEY_STRING, MSGQ_KEY_CHAR );
    int msgQId = getMessageQueue( msgQKey, 0760 );

    fd_set readfds;
    struct timeval tv;

    message msg; 
    char c, foo;
    int count = 1;

    say("Waiting for input");
    printf("\e[H\e[2KWaiting for input:");
    int cond = 1, res = 0;
    while( cond && (STORAGE_RUNNNING || FACTORY_RUNNING) ){
        if( msgrcv( msgQId, (void *)&msg, sizeof(message), WORKER_CLOSING_MSG_ID, IPC_NOWAIT ) != -1 ){
            success("Worker confirmed closed");
            printf("Worker confirmed closed\n");
            FACTORY_RUNNING--;
        }
        if( msgrcv( msgQId, (void *)&msg, sizeof(message), STORAGE_CLOSING_MSG_ID, IPC_NOWAIT ) != -1 ){
            success("Storage confirmed closed");
            printf("Storage confirmed closed\n");
            STORAGE_RUNNNING = 0;
        }
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        tv.tv_sec = 0;
        tv.tv_usec = 500000;
        if( (res = select(1, &readfds, NULL, NULL, &tv)) == -1){
            error("select");
            perror("select");
            exit(EXIT_FAILURE);
        }
        else if( !res || !FD_ISSET( STDIN_FILENO, &readfds ) )
            continue;
                
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
                cond = 0;
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