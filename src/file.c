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

    // request free block
    FatEntry* start_block = request_blocks(disk, 1);
    if (!start_block) {
            printf("Unable to create file: not enough blocks available!\n");
            return -1;
        }

    // save file on disk
    // FileHead* file = write_on_disk(disk);   // TODO
    // file->name = filename;
    // file->size = 0;
    // file->start = start_block;
    // file->is_dir = 0;

    // if (DEBUG) {
    //     printf("File %s created successfully\n", filename);
    //     printf("Name: %s\tSize: %d\tStart: %p", file->name, file->size, file->start);
    // }

    return 0;
}