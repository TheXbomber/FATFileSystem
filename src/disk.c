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
    char* buffer = (char*)mmap(NULL, DISK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, buf_fd, 0);
    if (!buffer)
        handle_error("error in disk_init -> mmap for buffer");
    //char* buffer = (char*)malloc(DISK_SIZE);
    buffer = memset(buffer, 0, DISK_SIZE);
    if (!buffer)
        handle_error("error in disk_init -> memset for buffer");
    // printf("BUFFER:\n");
    // for (int i = 0; buffer[i]; i++) {
    //     printf("%c", buffer[i]);
    // }
    // printf("\nEND\n");

    Disk* disk = (Disk*)buffer;
    disk->size = DISK_SIZE - FAT_SIZE - 8;
    if (DEBUG) printf("Disk size: %d\n", disk->size);
    disk->buffer = buffer;
    disk->start = buffer + FAT_SIZE + 8;
    disk->fat = fat_init(buffer);
    if (DEBUG)
        printf("Disk initalized correctly\n");
    return disk;
}

void disk_print(Disk* disk) {
    printf("----- DISK INFO -----\n");
    printf("--- FAT ---\nmax_blocks: %d\nfree_blocks: %d\ncontent:\n", disk->fat->max_blocks, disk->fat->free_blocks);
    for (int i = 8; i < FAT_SIZE; i++) {
        printf("n: %d\td: %d\tb: %c\n", i-8, disk->fat->array->data, disk->fat->array->busy);
    }
    printf("--- DISK ---\nsize: %d\nstart: %p\ncontent:\n", disk->size, disk->start);
    for (int i = FAT_SIZE+8; i < DISK_SIZE; i++) {
        printf("%c", disk->buffer[i]);
    }
    printf("\n----- END -----\n");
}