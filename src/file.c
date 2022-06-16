#include "error.h"
#include "file.h"
#include "disk.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

int create_file(Disk* disk, Dir* dir, char* filename) {
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
    FileHead* file = (FileHead*) find_block(disk);
    if (!file)
        printf("Unable to create file: cannot find a block in the disk!\n");
    FileHandle* handle = (FileHandle*) file;
    handle->pos = 0;
    file->handle = handle;
    file->is_dir = 0;
    file->name = filename;
    file->size = 0;
    file->parent_dir = dir;
    file->start = start_block;
    file->data = 0;

    if (DEBUG) {
        printf("File %s created successfully\n", filename);
        printf("Name: %s\tSize: %d\tStart: %p\n", file->name, file->size, file->start);
    }

    return 0;
}

int create_dir(Disk* disk, Dir* dir, char* dirname) {
    if (DEBUG)
        printf("Creating directory %s...\n", dirname);

    // request free block
    FatEntry* start_block = request_blocks(disk, 1);
    if (!start_block) {
        handle_error("unable to create directory: not enough blocks available!");
        return -1;
    }

    // save directory on disk
    Dir* new_dir = (Dir*) find_block(disk);
    if (!new_dir)
        handle_error("unable to create directory: cannot find a block in the disk!");

    new_dir->is_dir = 1;
    new_dir->name = dirname;
    new_dir->start = start_block;
    new_dir->parent_dir = dir;
    new_dir->num_files = 0;
    new_dir->files = NULL;
    
    if (!strcmp(dirname, "/"))
        dir = new_dir;

    if (DEBUG) {
        printf("Directory %s created successfully\n", dirname);
        // if (dir)
        //     printf("Name: %s\t Parent dir: %s\tStart: %p\n", dir->name, dir->parent_dir->name, dir->start);
        // NEEDS FIXING
    }

    return 0;
}

// TODO

// void read_file(char* filename) {
//     // if (file_exists(filename)) {    // TODO
//     //     printf("Unable to read file: file does not exist!\n");
//     //     return -1;
//     // }
//     FileHandle* handle = open_file(filename); // TODO

// }