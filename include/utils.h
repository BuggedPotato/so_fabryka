#pragma once
#include "types.h"

void say( char *text );
void warning( char *text );
void error( char *text );
void success( char *text );

int getStorage(key_t key);
char* attachStorage( int id );
int getSemaphores( key_t key, int count, int perms );
int getMessageQueue( key_t key, int perms );
key_t getKey( char* path, int id );
void getTime( char *dest );
int getStorageSegments( char* shmAddr, storageSegment *storageSegments );
void drawStorage( storageSegment *storage, int *position, int operation );