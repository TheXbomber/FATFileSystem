#include "headers/error.h"
#include "headers/disk.h"
#include "headers/file.h"
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
    
    Disk* disk = (Disk*) buffer;
    if (format)
        disk->size = DISK_SIZE - 3*sizeof(int) - sizeof(Fat);

    if (DEBUG) 
        printf("Disk size: %d\n", disk->size);

    if (format)
        fat_init(&disk->fat);

    if (DEBUG)
        printf("Disk initialized correctly at %p\n", disk);
    
    return disk;
}

void disk_print(Disk* disk) {
    printf("----- DISK INFO -----\n");
    printf("--- FAT ---\nFree_blocks: %d\nContent (-1 = file end, -2 = not used):\n", disk->fat.free_blocks);
    for (int i = 0; i < DISK_PRINT_LINES; i++) {
        // printf("Data: %d\t\tBusy: %d\t\tIdx: %d\t\tDisk block idx: %d\t\tAddr: %p\n", disk->fat.array[i].data, disk->fat.array[i].busy, disk->fat.array[i].idx, disk->fat.array[i].file, &(disk->fat.array[i]));
        printf("I: %d\t\tD: %d\t\tDI: %d\t\tA: %p\n", i, disk->fat.array[i].data, disk->fat.array[i].file, &(disk->fat.array[i]));
    }
    printf("--- DISK ---\nDisk size: %d\n", disk->size);
    printf("----- END -----\n");
}

FatEntry* request_fat_blocks(Disk* disk, FatEntry* prev, int n_blocks) {
    if (!disk)
        handle_error("error in request block (disk error)");
    if (disk->fat.free_blocks < n_blocks)
        return NULL;
    if (DEBUG)
        printf("Requesting blocks...\n");
    int allocated = 0;
    FatEntry* first;
    FatEntry* previous;
    if (prev)
        previous = prev;
    for (int i = 0; i < FAT_BLOCKS_MAX; i++) {
        if (!(disk->fat.array[i].data != -2)) {
            //disk->fat.array[i].busy = 1;
            disk->fat.array[i].data = -1;
            if (allocated || prev) {
                previous->data = get_fat_entry_idx(disk, &disk->fat.array[i]);
                if (previous->data == -1) {
                    printf("Error getting FAT entry index!\n");
                    return NULL;
                }
            }
            allocated++;
            disk->fat.free_blocks--;
            // what will be returned
            if (allocated == 1)
                first = &(disk->fat.array[i]);
            if (allocated == n_blocks) {
                if (DEBUG)
                    printf("Successfully allocated %d blocks starting at %p\n", allocated, first);
                return first;
            }
            // needed for next iteration
            previous = &(disk->fat.array[i]);
        }
    }
    return NULL;
}

char* find_block(Disk* disk) {
    char* start = disk->data;
    char* current = start;
    while (*current) {
        current += BLOCK_SIZE;
    }
    if (DEBUG)
        printf("Location on disk: %p\n", current);
    return current;
}

int get_block_idx(Disk* disk) {
    int i = 0;
    char* current = disk->data;
    while (*current) {
        current += BLOCK_SIZE;
        i += BLOCK_SIZE;
    }
    return (i / BLOCK_SIZE) + 1;
}