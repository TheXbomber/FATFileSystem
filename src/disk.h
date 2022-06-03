#pragma once
#include "fat.h"

#define BLOCK_SIZE 256
#define DISK_SIZE 1048576

typedef struct {
    Fat* fat;       // pointer to start of the FAT
    int size;       // size of the disk array
    char* start;    // pointer to start of the disk array
    char* buffer;   // buffer on which the disk is mapped
} Disk;

Disk* disk_init(char* buffer);
void disk_print(Disk* disk);