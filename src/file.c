#include "error.h"
#include "file.h"
#include "disk.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

int create_file(Disk* disk, Dir* parent_dir, char* filename) {
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
    file->name = filename;
    file->is_dir = 0;
    file->size = 0;
    file->pos = 0;
    file->parent_dir = parent_dir;
    parent_dir->num_files++;
    for (int i = 0; i < parent_dir->num_files; i++) {
        if (!parent_dir->files[i]) {
            parent_dir->files[i] = file;
            if (DEBUG)
                printf("Inserted file %s into dir %s\n", parent_dir->files[i]->name, parent_dir->name);
            break;
        }
    }
    file->start = start_block;
    file->data = 0;

    if (DEBUG) {
        printf("File %s created successfully\n", filename);
        printf("Name: %s\tParent directory: %s\tSize: %d\tStart: %p\n", file->name, file->parent_dir->name, file->size, file->start);
    }

    return 0;
}

Dir* create_dir(Disk* disk, Dir* parent_dir, char* dirname) {
    if (DEBUG)
        printf("Creating directory %s...\n", dirname);

    // request free block
    FatEntry* start_block = request_blocks(disk, 1);
    if (!start_block) {
        handle_error("unable to create directory: not enough blocks available!");
        return NULL;
    }

    // save directory on disk
    Dir* new_dir = (Dir*) find_block(disk);
    if (!new_dir)
        handle_error("unable to create directory: cannot find a block in the disk!");

    new_dir->is_dir = 1;
    new_dir->name = dirname;
    new_dir->start = start_block;
    new_dir->parent_dir = parent_dir;
    new_dir->num_files = 0;
    new_dir->files = (FileHead**) (new_dir + sizeof(Dir));

    if (DEBUG) {
        printf("Directory %s created successfully\n", dirname);
        if (parent_dir)
            printf("Name: %s\t Parent dir: %s\tStart: %p\n", new_dir->name, new_dir->parent_dir->name, new_dir->start);
    }

    return new_dir;
}

int list_dir(Dir* dir) {
    printf("Content of %s:\n", dir->name);
    printf("Dir\tName\tSize\n");
    int i;
    for(i = 0; i < dir->num_files; i++) {
        printf("%d\t%s\t%d\n", dir->files[i]->is_dir, dir->files[i]->name, dir->files[i]->size);
    }
    return i;
}

FileHead* open_file(char* filename, Dir* cur_dir, Disk* disk) {
    // 1) Look for the file in the FAT
    // 2) Return the FileHead
    // 3) Return the FileHandle
    if (DEBUG)
        printf("Opening file %s\n", filename);
    FileHead* head;
    char* name;
    for (int i = 0; i < cur_dir->num_files; i++) {
        name = cur_dir->files[i]->name;
        if (!strcmp(filename, name)) {
            head = cur_dir->files[i];
            return head;
        }
    }
    printf("File not present in the current_directory!\n");
    return NULL;
}

int read_file(char* filename, Dir* cur_dir, Disk* disk) {
    // if (file_exists(filename)) {    // TODO
    //     printf("Unable to read file: file does not exist!\n");
    //     return -1;
    // }
    if (DEBUG)
        printf("Reading file %s\n", filename);
    FileHead* head = open_file(filename, cur_dir, disk);
    if (!head)
        handle_error("error opening file!");
    
    printf("Content of %s:\n", filename);
    int n_bytes = head->size;
    int i = 0;
    for (i = 0; i < n_bytes; i++) {
        printf("%c", head->data[i]);
    }
    printf("\nEnd of content\n");
    return i;
}