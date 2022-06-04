#pragma once
#include "fat.h"

#define BLOCK_SIZE 256
#define DISK_SIZE 1048576

typedef struct {
    int size;       // size of the disk array
    char* start;    // pointer to start of the disk array
    Fat* fat;       // pointer to start of the FAT
} Disk;

// initialize disk from file
Disk* disk_init(char* filename, char* buffer);

// print info on disk
void disk_print(Disk* disk, char* buffer);

// requst n_blocks blocks
FatEntry* request_blocks(Disk* disk, int n_blocks);   // TODO