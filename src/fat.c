#include "error.h"
#include "fat.h"
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>

Fat* fat_init(char* buffer) {
    if (DEBUG)
        printf("Initializing FAT...\n");
    if (DEBUG) printf("FAT size: %d\n", FAT_SIZE);
    // Fat* fat = mmap(buffer, FAT_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS, -1, 0);
    Fat* fat = (Fat*)buffer;
    fat->max_blocks = FAT_BLOCKS_MAX;
    if (DEBUG) printf("FAT max blocks: %d\n", fat->max_blocks);
    fat->free_blocks = fat->max_blocks;
    if (DEBUG) printf("FAT free blocks: %d\n", fat->free_blocks);
    fat->array = (FatEntry*)(fat + 8);
    for (int i = 0; i < FAT_BLOCKS_MAX; i++) {  // initialize empty FAT
        fat->array[i].data = -1;
        fat->array[i].busy = 0;
    }
    return fat;
}