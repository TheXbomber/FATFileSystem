#pragma once

#define FAT_BLOCKS_MAX 256
#define FAT_SIZE sizeof(Fat)+FAT_BLOCKS_MAX*sizeof(FatEntry)

typedef struct {
    int data;               // index of next block
    int busy;              // 0 if the block is free
} FatEntry;

typedef struct {
    int free_blocks;        // number of free blocks
    FatEntry* array;        // FAT
} Fat;

Fat* fat_init(char* buffer);