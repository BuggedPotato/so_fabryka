#pragma once
#include "constants.h"

typedef struct memInterval{
    char *start;
    char *end;
    // indexes to read from and write to
    int *read;
    int *write;
    int elSize;
} storageSegment;

typedef struct message {
    long int type;
    char text[MSG_TEXT_SIZE];
} message;