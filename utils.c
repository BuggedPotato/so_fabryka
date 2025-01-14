#include<sys/stat.h>

int fileExists(char *fileName){
    struct stat buffer;
    return (stat(fileName, &buffer) == 0);
}