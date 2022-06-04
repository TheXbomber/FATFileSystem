#pragma once

#define FAT_BLOCKS_MAX 256
#define FAT_SIZE FAT_BLOCKS_MAX*sizeof(FatEntry)+sizeof(Fat)

typedef struct {
    char busy;              // 0 if the block is free
    int data;               // index of next block
} FatEntry;

typedef struct {
    int free_blocks;        // number of free blocks
    FatEntry* array;        // FAT
} Fat;

Fat* fat_init(char* buffer);