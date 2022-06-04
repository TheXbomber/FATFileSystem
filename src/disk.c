#include "error.h"
#include "disk.h"
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

Disk* disk_init(char* filename, char* buffer) {
    if (DEBUG)
        printf("Initializing disk...\n");
    int buf_fd = open(filename, O_CREAT | O_RDWR, 0666);
    if (!buf_fd)
        handle_error("error in disk_init/open");
    int ret = ftruncate(buf_fd, DISK_SIZE);
    if (ret)
        handle_error("error in disk_init/ftruncate");
    buffer = (char*)mmap(NULL, DISK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, buf_fd, 0);
    if (!buffer)
        handle_error("error in disk_init -> mmap for buffer");
    //char* buffer = (char*)malloc(DISK_SIZE);
    buffer = memset(buffer, 0, DISK_SIZE);
    if (!buffer)
        handle_error("error in disk_init -> memset for buffer");
    
    // if (DEBUG) {
    //     printf("BUFFER:\n");
    //     for (int i = 0; buffer[i]; i++) {
    //         printf("%c", buffer[i]);
    //     }
    //     printf("\nEND\n");
    // }

    Disk* disk = malloc(sizeof(Disk));
    disk->fat = fat_init(buffer);
    disk->size = DISK_SIZE;
    if (DEBUG) 
        printf("Disk size: %d\n", disk->size);
    disk->start = buffer;
    if (DEBUG)
        printf("Disk initialized correctly at %p\n", disk);
    // if (DEBUG) {
    //     printf("BUFFER:\n");
    //     for (int i = 0; buffer[i]; i++) {
    //         printf("%c", buffer[i]);
    //     }
    //     printf("\nEND\n");
    // }
    
    return disk;
}

void disk_print(Disk* disk, char* buffer) {
    printf("----- DISK INFO -----\n");
    printf("--- FAT ---\nfree_blocks: %d\ncontent:\n", disk->fat->free_blocks);
    for (int i = 0; i < FAT_BLOCKS_MAX; i++) {
        printf("n: %d\td: %d\tb: %c\n", i, disk->fat->array[i].data, disk->fat->array[i].busy);
    }
    printf("--- DISK ---\nsize: %d\nstart: %p\ncontent:\n", disk->size, disk->start);
    for (int i = 0; i < DISK_SIZE; i++) {
        printf("%c", *((disk->start)+i));
    }
    printf("\n----- END -----\n");
}

FatEntry* request_blocks(Disk* disk, int n_blocks) {
    if (!disk)
        handle_error("error in request block (disk error)");
    if (disk->fat->free_blocks < n_blocks)
        return NULL;
    if (DEBUG)
        printf("Requesting blocks...\n");
    int allocated = 0;
    FatEntry* first;
    int prev_idx;
    for (int i = 0; i < FAT_BLOCKS_MAX; i++) {
        if (!(disk->fat->array[i].busy)) {
            if (allocated) {
                if (allocated == 1)
                    first->data = i;
                disk->fat->array[prev_idx].data = i;
            }
            disk->fat->array[i].busy = 1;
            allocated ++;
            prev_idx = i;
            if (allocated == 1)
                first = &(disk->fat->array[i]);
            if (allocated == n_blocks) {
                if (DEBUG)
                    printf("Successfully allocated %d blocks starting at %p\n", allocated, first);
                return first;
            }
        }
    }
    return NULL;
}