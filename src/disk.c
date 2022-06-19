#include "error.h"
#include "disk.h"
#include "file.h"
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

char* map_file(char* filename) {
    if (DEBUG)
        printf("Mapping file...\n");
    int buf_fd = open(filename, O_CREAT | O_RDWR, 0666);
    if (!buf_fd)
        handle_error("error in map_file/open");
    int ret = ftruncate(buf_fd, DISK_SIZE);
    if (ret)
        handle_error("error in map_file/ftruncate");
    char* buffer = (char*) mmap(NULL, DISK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, buf_fd, 0);
    if (!buffer)
        handle_error("error in map_file -> mmap for buffer");
    if (DEBUG)
        printf("Succesfully mapped zone %p -> %p\n", buffer, buffer + DISK_SIZE);
    return buffer;
}

Disk* disk_init(char* buffer, int format) {
    if (DEBUG)
        printf("Initializing disk...\n");
    if (format) {
        buffer = (char*) memset(buffer, 0, DISK_SIZE);
        if (!buffer)
            handle_error("error in disk_init -> memset for buffer");
    }
    
    if (DEBUG) {
        printf("BUFFER:\n");
        for (int i = 0; buffer[i]; i++) {
            printf("%c", buffer[i]);
        }
        printf("\nEND\n");
    }
    
    Disk* disk = (Disk*) buffer;
    int fatsize = FAT_SIZE;
    disk->size = DISK_SIZE - sizeof(Disk) - fatsize;
    if (DEBUG) 
        printf("Disk size: %d\n", disk->size);

    disk->fat = (Fat*) (buffer + sizeof(Disk));
    fat_init(disk->fat);
    //printf("First FAT: %p, %d, %d\n", disk->fat->array, disk->fat->array[0].data, disk->fat->array[0].busy);

    if (DEBUG)
        printf("Disk initialized correctly at %p\n", disk);
    
    return disk;
}

void disk_print(Disk* disk) {
    printf("\n----- DISK INFO -----\n");
    printf("--- FAT ---\nfree_blocks: %d\ncontent:\n", disk->fat->free_blocks);
    for (int i = 0; i < FAT_BLOCKS_MAX; i++) {
        printf("n: %d\td: %d\tb: %d\ti: %d\ta: %p\n", i, disk->fat->array[i].data, disk->fat->array[i].busy, disk->fat->array[i].idx, &(disk->fat->array[i]));
    }
    printf("----- END -----\n\n");
}

FatEntry* request_fat_blocks(Disk* disk, FatEntry* prev, int n_blocks) {
    if (!disk)
        handle_error("error in request block (disk error)");
    if (disk->fat->free_blocks < n_blocks)
        return NULL;
    if (DEBUG)
        printf("Requesting blocks...\n");
    int allocated = 0;
    FatEntry* first;
    FatEntry* previous;
    if (prev)
        previous = prev;
    for (int i = 0; i < FAT_BLOCKS_MAX; i++) {
        if (!(disk->fat->array[i].busy)) {
            disk->fat->array[i].busy = 1;
            if (allocated || prev)
                previous->data = disk->fat->array[i].idx;
            allocated++;
            disk->fat->free_blocks--;
            // what will be returned
            if (allocated == 1)
                first = &(disk->fat->array[i]);
            if (allocated == n_blocks) {
                if (DEBUG)
                    printf("Successfully allocated %d blocks starting at %p\n", allocated, first);
                return first;
            }
            // needed for next iteration
            previous = &(disk->fat->array[i]);
        }
    }
    return NULL;
}

char* find_block(Disk* disk) {
    char* start = (char*) (disk->fat + FAT_SIZE);
    char* current = start;
    while (*current) {
        current += BLOCK_SIZE;
    }
    if (DEBUG)
        printf("Location on disk: %p\n", current);
    return current;
}