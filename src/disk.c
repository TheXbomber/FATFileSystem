#include "error.h"
#include "disk.h"
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>

Disk* disk_init(char* filename) {
    if (DEBUG)
        printf("Initializing disk...\n");
    int buf_fd = open(filename, O_CREAT | O_RDWR, 0666);    // ...
    if (!buf_fd)
        handle_error("error in disk_init/open");
    char* buffer = mmap(NULL, DISK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, buf_fd, 0);
    if (!buffer)
        handle_error("error in disk_init/mmap");
    Disk* disk = (Disk*)buffer;
    disk->size = DISK_SIZE - FAT_SIZE - 8;
    disk->buffer = buffer;
    disk->start = buffer + FAT_SIZE + 8;
    disk->fat = fat_init(buffer);
    if (DEBUG)
        printf("Disk initalized correctly\n");
    return disk;
}

void disk_print(Disk* disk) {
    printf("----- DISK INFO -----\n");
    printf("FAT:\n\
            max_blocks: %d\n\
            free_blocks: %d\n\
            content:\n",
            disk->fat->max_blocks,
            disk->fat->free_blocks);
    for (int i = 8; i < FAT_SIZE; i++) {
        printf("d: %d\tb: %c", disk->fat->array->data, disk->fat->array->busy);
    }
    printf("DISK:\n\
            size: %d\n\
            start: %p\n\
            content:\n",
            disk->size,
            disk->start);
    for (int i = FAT_SIZE+8; i < DISK_SIZE; i++) {
        printf("%c\n", disk->buffer[i]);
    }
    printf("----- END -----\n");
}