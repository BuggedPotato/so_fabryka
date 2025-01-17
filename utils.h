#pragma once
#include "types.h"

int fileExists(char *fileName);
void say( char *text );
void warning( char *text );
void error( char *text );

key_t getKey( char* path, int id );
int getStorage(key_t key);
char* attachStorage( int id );
int getSemaphores( key_t key, int count, int perms );
int getMessageQueue( key_t key, int perms );

int getStorageSegments( char* shmAddr, storageSegment *storageSegments );

int processExists( pid_t pid );