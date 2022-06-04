#include "error.h"
#include "fat.h"
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <fcntl.h>

Fat* fat_init(char* buffer) {
    if (DEBUG)
        printf("Initializing FAT...\nFAT size: %ld\n", FAT_SIZE);

    Fat* fat = malloc(sizeof(Fat));
    fat->free_blocks = FAT_BLOCKS_MAX;
    if (DEBUG) 
        printf("FAT free blocks: %d\n", fat->free_blocks);
    fat->array = malloc(sizeof(FatEntry*));
    for (int i = 0; i < FAT_BLOCKS_MAX; i++) {  // initialize empty FAT
        fat->array[i].data = -1;
        fat->array[i].busy = 0;
        // if (DEBUG) {
        //     printf("n: %d\td: %d\tb: %c\n", i, fat->array[i].data, fat->array[i].busy);
        // }
    }
    if (DEBUG)
        printf("FAT initalized correctly at %p\n", fat);
    return fat;
}