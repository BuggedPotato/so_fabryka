#pragma once
#include<stdio.h>
#include<time.h>

#define DEBUG 0
#define VERBOSE 0

#define SPEED NO_SLEEP
#define SLOW 2
#define FAST 1
#define NO_SLEEP 0

#define SIZE_X 1
#define SIZE_Y 2
#define SIZE_Z 3

#define STORAGE_COUNT 16
#define STORAGE_SIZE (STORAGE_COUNT * (SIZE_X + SIZE_Y + SIZE_Z))
#define STORAGE_TOTAL_SIZE (STORAGE_SIZE + 6 * sizeof(int))
#define WORKERS 2
#define DELIVERIES 3
#define MSG_TEXT_SIZE 32

#define SEM_STORAGE 0
// #define SEM_WORKERS 0
#define SEM_DELIVERY 1
#define SEM_QUEUE 2

#define STORAGE_FILENAME "storage_dump"

#define STORAGE_KEY_STR "./src/storage.c"
#define STORAGE_KEY_CHAR 'S'
#define SEM_KEY_STR "./src/delivery.c"
#define SEM_KEY_CHAR 'D'
#define MSGQ_KEY_STRING "./src/director.c"
#define MSGQ_KEY_CHAR 'M'


#define MESSAGES_STORAGE -3
#define MESSAGES_WORKERS -2 // ALSO NEEDS CHECK FOR POLECENIE_2

#define POLECENIE_1_MSG_ID 3
#define POLECENIE_2_MSG_ID 4
#define POLECENIE_3_MSG_ID 1
#define POLECENIE_4_MSG_ID 2
#define WORKER_CLOSING_MSG_ID 5
#define STORAGE_CLOSING_MSG_ID 6

#define BADGE_MANAGER "MANAGER"
#define BADGE_STORAGE "STORAGE"
#define BADGE_DIRECTOR "DIRECTOR"

#define LOG_NAME "log"
// FILE *LOG_FILE;