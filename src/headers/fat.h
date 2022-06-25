#pragma once

#define FAT_BLOCKS_MAX 256
#define FAT_SIZE sizeof(Fat)+FAT_BLOCKS_MAX*sizeof(FatEntry)

typedef struct File File;

typedef struct {
    int file;               // index of the disk block
    int data;               // index of next block
    int busy;               // 0 if the block is free
    int idx;                // index of this block
} FatEntry;

typedef struct {
    int free_blocks;                        // number of free blocks
    FatEntry array[FAT_BLOCKS_MAX];         // FAT
} Fat;

// initializes the FAT structure
void fat_init(Fat* fat);