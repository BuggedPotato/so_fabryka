#pragma once

#define DEBUG 0

#define SIZE_X 1
#define SIZE_Y 2
#define SIZE_Z 3

#define STORAGE_COUNT 15
#define STORAGE_SIZE (STORAGE_COUNT * (SIZE_X + SIZE_Y + SIZE_Z))
#define STORAGE_TOTAL_SIZE (STORAGE_SIZE + 6 * sizeof(int))
#define WORKERS 2
#define DELIVERIES 3
#define MSG_TEXT_SIZE 32

#define SEM_WORKERS 0
#define SEM_DELIVERY 1

#define STORAGE_FILENAME "storage_dump"

#define STORAGE_KEY_STR "./storage.c"
#define STORAGE_KEY_CHAR 'S'
#define SEM_KEY_STR "./delivery.c"
#define SEM_KEY_CHAR 'D'
#define MSGQ_KEY_STRING "./director.c"
#define MSGQ_KEY_CHAR 'M'


#define MESSAGES_STORAGE -3
#define MESSAGES_WORKERS -2 // ALSO NEEDS CHECK FOR POLECENIE_2

#define POLECENIE_1_MSG_ID 3
#define POLECENIE_2_MSG_ID 4
#define POLECENIE_3_MSG_ID 1
#define POLECENIE_4_MSG_ID 2

#define BADGE_MANAGER "MANAGER"
#define BADGE_STORAGE "STORAGE"
#define BADGE_DIRECTOR "DIRECTOR"