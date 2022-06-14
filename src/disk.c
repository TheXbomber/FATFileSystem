#include "error.h"
#include "disk.h"
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

Disk* disk_init(char* filename) {
    if (DEBUG)
        printf("Initializing disk...\n");
    int buf_fd = open(filename, O_CREAT | O_RDWR, 0666);
    if (!buf_fd)
        handle_error("error in disk_init/open");
    int ret = ftruncate(buf_fd, DISK_SIZE);
    if (ret)
        handle_error("error in disk_init/ftruncate");
    char* buffer = (char*) mmap(NULL, DISK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, buf_fd, 0);
    if (!buffer)
        handle_error("error in disk_init -> mmap for buffer");
    if (DEBUG)
        printf("Succesfully mapped zone %p -> %p\n", buffer, buffer + DISK_SIZE);
    //char* buffer = (char*)malloc(DISK_SIZE);
    buffer = (char*) memset(buffer, 0, DISK_SIZE);
    if (!buffer)
        handle_error("error in disk_init -> memset for buffer");
    
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
    // if (DEBUG) {
    //     printf("BUFFER:\n");
    //     for (int i = 0; buffer[i]; i++) {
    //         printf("%c", buffer[i]);
    //     }
    //     printf("\nEND\n");
    // }
    
    return disk;
}

void disk_print(Disk* disk) {
    printf("----- DISK INFO -----\n");
    printf("--- FAT ---\nfree_blocks: %d\ncontent:\n", disk->fat->free_blocks);
    for (int i = 0; i < FAT_BLOCKS_MAX; i++) {
        printf("n: %d\td: %d\tb: %d\n", i, disk->fat->array[i].data, disk->fat->array[i].busy);
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
            disk->fat->free_blocks--;
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