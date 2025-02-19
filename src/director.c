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
// how many messages of each type were sent
int POLECENIA_COUNT[4] = {0,0,0,0};

int deleteMessageQueue( int msgQId );
int sendMessage( int msgQId, message *msg );

int main(int argc, char *argv[]){
    PID = getpid();
    say("Started");
    // ignoring SIGCHLD to not have zombies
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
        if( msgrcv( msgQId, (void *)&msg, MSG_TEXT_SIZE, WORKER_CLOSING_MSG_ID, IPC_NOWAIT ) != -1 ){
            success("Worker confirmed closed");
            printf("Worker confirmed closed\n");
            FACTORY_RUNNING--;
        }
        if( msgrcv( msgQId, (void *)&msg, MSG_TEXT_SIZE, STORAGE_CLOSING_MSG_ID, IPC_NOWAIT ) != -1 ){
            success("Storage confirmed closed");
            printf("Storage confirmed closed\n");
            STORAGE_RUNNNING = 0;
        }
        // periodically check for input on stdin if none skip
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
                
        // send message based on input
        c = fgetc(stdin);
        while ((foo = getchar()) != '\n' && foo != EOF);
        count = 1;
        switch (c)
        {
            case '1':
                msg.type = POLECENIE_1_MSG_ID;
                // POLECENIA_COUNT[0]++;
                break;
            case '2':
                msg.type = POLECENIE_2_MSG_ID;
                count = WORKERS;
                // POLECENIA_COUNT[1] += WORKERS;
                break;
            case '3':
                msg.type = POLECENIE_3_MSG_ID;
                count = WORKERS + 1;
                // POLECENIA_COUNT[2] += WORKERS + 1;
                break;
            case '4':
                msg.type = POLECENIE_4_MSG_ID;
                count = WORKERS + 1;
                // POLECENIA_COUNT[3] += WORKERS + 1;
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
/*
* Function Name:	sendMessage
*
* Function:			send a message to message queue while checking for overflow and 
                    removing unnecessary messages from queue
*
* Arguments:		id - message queue id,
                    msg - pointer to message object
*
* Return:			0->success, othewise exit with errno
*/
int sendMessage( int id, message *msg ){
    if( msgsnd( id, (void *)msg, MSG_TEXT_SIZE, IPC_NOWAIT ) == -1 ){
        if( errno == EAGAIN ){
            // msgq is full
            int limit[4] = {1, WORKERS, WORKERS+1, WORKERS+1};
            int msgIds[4] = {POLECENIE_1_MSG_ID, POLECENIE_2_MSG_ID, POLECENIE_3_MSG_ID, POLECENIE_4_MSG_ID};
            // check limits for each message id and remove unwanted
            for( int i = 0; i < 4; i++ ){
                while(POLECENIA_COUNT[i] > limit[i]){
                    fprintf(stderr, "%d\n", POLECENIA_COUNT[i]);
                    message foo;
                    if( msgrcv( id, (void *)&foo, MSG_TEXT_SIZE, msgIds[i], IPC_NOWAIT ) == -1 ){
                        if( errno == ENOMSG ){
                            error("No such message in the queue!");
                            POLECENIA_COUNT[i] = 0;
                            break;
                        }
                        else{
                            perror("");
                            fprintf(stderr, "%d\n", errno);
                            exit(errno);
                        }
                    }
                    #if DEBUG
                        fprintf(stderr, "Queue full - removed unnecessary message with id %d\n", msgIds[i]);
                    #endif
                    POLECENIA_COUNT[i]--;
                }
            }
            //try sending again
            sendMessage(id, msg);
        }
        else{
            perror("error sending message");
            exit(errno);
        }
    }
    int index[5] = { -1, 2, 3, 0, 1};
    POLECENIA_COUNT[index[msg->type]]++;
    // error("end");
    return 0;
}

/*
* Function Name:	deleteMessageQueue
*
* Function:			deletes message queue
*
* Arguments:		id - message queue id,
*
* Return:			0->succes, othewise exit with errno
*/
int deleteMessageQueue( int id ){
    if( msgctl( id, IPC_RMID, NULL ) == -1 ){
        perror("error deleting message queue");
        exit(errno);
    }
    say("Message queue successfully deleted");
    return 0;
}
