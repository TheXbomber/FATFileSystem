#include "error.h"
#include "file.h"
#include "disk.h"
#include <stdio.h>
#include <math.h>

int create_file(Disk* disk, char* filename) {
    // 1) Check if file already exists
    // 2) Request free block(s)
    // 3) Allocate block(s)

    // if (file_exists(filename)) {    // TODO
    //     printf("Unable to create file: file already exists!\n");
    //     return -1;
    // }

    if (DEBUG)
        printf("Creating file %s...\n", filename);
    File* file = (File*)malloc(sizeof(File)); // ERROR
    file->name = filename;
    file->size = 0;

    // request free block(s)
    file->num_blocks = 1;
    FatEntry* start_block = request_blocks(disk, file->num_blocks); // TODO
    if (!start_block) {
        printf("Unable to create file: not enough blocks available!\n");
        return -1;
    }
    file->start = start_block;

    file->path = "/";   // TODO
    file->is_dir = 0;
    file->data = 0;

    if (DEBUG) {
        printf("File %s created successfully\n", filename);
        printf("Name: %s\tSize: %d\tStart: %p\tNum blocks: %d\tPath: %s", file->name, file->size, file->start, file->num_blocks, file->path);
    }

    return 0;
}