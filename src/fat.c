#include "error.h"
#include "fat.h"
#include <stdio.h>

Fat* fat_init(char* buffer) {
    if (DEBUG)
        printf("Initializing FAT...\n");
    Fat* fat = buffer;
    fat->max_blocks = FAT_BLOCKS_MAX;
    fat->free_blocks = fat->max_blocks;
    fat->array = (FatEntry*) (buffer + 8);
    for (int i = 0; i < FAT_BLOCKS_MAX; i++) {  // initialize empty FAT
        fat->array[i].data = -1;
        fat->array[i].busy = 0;
    }
    return fat;
}