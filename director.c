#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<errno.h>
#include "constants.h"
#include "utils.h"
#include "types.h"
pid_t PID;

int main(int argc, char *argv[]){
    PID = getpid();
    key_t msgQKey = getKey( "./director.c", 'M' );
    int msgQId = getMessageQueue( msgQKey, 0700 );
    message msg; 
    msg.type = POLECENIE_1_MSG_ID;
    sleep(5);
    say("Sending message Polecenie_1");
    if( msgsnd( msgQId, (void *)&msg, MSG_TEXT_SIZE, 0 ) == -1 ){
        perror("error sending message");
        exit(errno);
    }
    say("Message sent!");
    say("Shutting downn...");

    return 0;
}

