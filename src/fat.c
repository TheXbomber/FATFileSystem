#include "error.h"
#include "fat.h"
#include "file.h"
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <fcntl.h>

void fat_init(Fat* fat) {
    if (DEBUG)
        printf("Initializing FAT...\nFAT size: %ld\n", FAT_SIZE);

    if (DEBUG)
        printf("FAT pointer is at %p\n", fat);
    fat->free_blocks = FAT_BLOCKS_MAX;
    if (DEBUG) 
        printf("FAT free blocks: %d\n", fat->free_blocks);
    fat->array = (FatEntry*) (fat + sizeof(Fat));
    if (DEBUG)
        printf("FAT starts at %p\n", fat->array);
    for (int i = 0; i < FAT_BLOCKS_MAX; i++) {  // initialize empty FAT
        fat->array[i].data = -1;
        fat->array[i].busy = 0;
        fat->array[i].idx = i;
        fat->array[i].file = (File*)fat + sizeof(Fat);
    }
    //printf("First FAT: %p, %d, %d\n", &(fat->array[0]), fat->array[0].data, fat->array[0].busy);
    if (DEBUG)
        printf("FAT initalized correctly at %p\n", fat);
}