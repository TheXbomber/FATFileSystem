#include "error.h"
#include "fat.h"
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <fcntl.h>

Fat* fat_init(char* buffer) {
    if (DEBUG)
        printf("Initializing FAT...\nFAT size: %ld\n", FAT_SIZE);

    Fat* fat = (Fat*) buffer;
    if (DEBUG)
        printf("FAT pointer is at %p\n", fat);
    fat->free_blocks = FAT_BLOCKS_MAX;
    if (DEBUG) 
        printf("FAT free blocks: %d\n", fat->free_blocks);
    fat->array = (FatEntry*) (buffer + sizeof(Fat)+8);
    if (DEBUG)
        printf("FAT starts at %p\n", fat->array);
    if (DEBUG)
        printf("Sizeof(FatEntry): %ld\n", sizeof(FatEntry));
    for (int i = 0; i < FAT_BLOCKS_MAX; i++) {  // initialize empty FAT
        // if (DEBUG)
        //     printf("Accessing position %p\n", &(fat->array[i]));
        fat->array[i].data = -1;
        fat->array[i].busy = 0;
        // if (DEBUG) {
        //     printf("n: %d\td: %d\tb: %c\n", i, fat->array[i].data, fat->array[i].busy);
        // }
    }
    printf("First FAT: %p, %d, %d\n", &(fat->array[0]), fat->array[0].data, fat->array[0].busy);
    if (DEBUG)
        printf("FAT initalized correctly at %p\n", fat);
    return fat;
}