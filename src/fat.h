#pragma once

#define FAT_BLOCKS_MAX 256
#define FAT_SIZE FAT_BLOCKS_MAX*5+8

typedef struct {
    int data;               // index of next block
    char busy;              // 0 if the block is free
} FatEntry;

typedef struct {
    int max_blocks;
    int free_blocks;
    FatEntry* array;   // FAT
} Fat;

Fat* fat_init(char* buffer);