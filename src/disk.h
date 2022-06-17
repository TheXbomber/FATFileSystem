#pragma once
#include "fat.h"

#define BLOCK_SIZE 512
#define DISK_SIZE 1048576

typedef struct {
    int size;       // size of the disk array
    Fat* fat;       // pointer to start of the FAT
} Disk;


char* map_file(char* filename);

// initialize disk from file
Disk* disk_init(char* buffer, int format);

// print info on disk
void disk_print(Disk* disk);

// requst n_blocks blocks
FatEntry* request_blocks(Disk* disk, int n_blocks);   

// finds the first available space on the disk (in increments of BLOCK_SIZE)
char* find_block(Disk* disk);