#pragma once
#include "fat.h"

#define BLOCK_SIZE 512
#define DISK_SIZE 1048576

typedef struct Dir Dir;

typedef struct {
    int size;       // size of the disk array
    Dir* cur_dir;   // pointer to current directory
    Fat* fat;       // pointer to start of the FAT
} Disk;


char* map_file(char* filename);

// initializes disk from file and returns the pointer to it
Disk* disk_init(char* buffer, int format);

// prints info about the disk
void disk_print(Disk* disk);

// requsts n_blocks blocks and returns the pointer to the first
FatEntry* request_fat_blocks(Disk* disk, FatEntry* prev, int n_blocks);   

// finds the first available space on the disk (in increments of BLOCK_SIZE) and returns a pointer to it
char* find_block(Disk* disk);