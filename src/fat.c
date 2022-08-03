#include "headers/error.h"
#include "headers/fat.h"
#include "headers/file.h"
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
    if (DEBUG)
        printf("FAT starts at %p\n", fat->array);
        
    for (int i = 0; i < FAT_BLOCKS_MAX; i++) {  // initialize empty FAT
        fat->array[i].data = -1;
        fat->array[i].busy = 0;
        //fat->array[i].idx = i;
    }
    if (DEBUG)
        printf("FAT initalized correctly at %p\n", fat);
}

int get_fat_entry_idx(Disk* disk, FatEntry* fatentry) {
    Fat* fat = &disk->fat;
    for (int i = 0; i < FAT_BLOCKS_MAX; i++) {
        FatEntry* fe = &fat->array[i];
        if (fatentry->file == fe->file && fatentry->data == fe->data)
            return i;
    }
    return -1;
}