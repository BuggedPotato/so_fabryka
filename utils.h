#pragma once

int fileExists(char *fileName);
void say( char *text );

key_t getKey( char* path, int id );
int getSemaphores( key_t key, int count, int perms );
int getMessageQueue( key_t key, int perms );