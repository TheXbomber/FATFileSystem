#include "error.h"
#include "file.h"
#include "disk.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

int file_exists(char* filename, Dir* cur_dir) {
    for (int i = 0; i < cur_dir->num_files; i++) {
        if (!strcmp(cur_dir->files[i]->name, filename)) {
            return 1;
        }
    }
    return 0;
}

int dir_exists(char* dirname, Dir* cur_dir) {
    for (int i = 0; i < cur_dir->num_dirs; i++) {
        if (!strcmp(cur_dir->dirs[i]->name, dirname)) {
            return 1;
        }
    }
    return 0;
}

int create_file(Disk* disk, Dir* parent_dir, char* filename) {
    // 1) Check if file already exists
    // 2) Request free block(s)
    // 3) Allocate block(s)

    if (DEBUG)
        printf("Creating file %s...\n", filename);
    if (file_exists(filename, parent_dir)) {
        printf("Unable to create file: file already exists!\n");
        return -1;
    }

    // request free block
    FatEntry* start_block = request_blocks(disk, 1);
    if (!start_block) {
        printf("Unable to create file: not enough blocks available!\n");
        return -1;
    }

    // save file on disk
    FileHead* head = (FileHead*) find_block(disk);
    if (!head)
        printf("Unable to create file: cannot find a block in the disk!\n");
    head->name = filename;
    head->is_dir = 0;
    head->size = 0;
    //file->free_in_block = BLOCK_SIZE - sizeof(FileHead);
    head->pos = 0;
    head->parent_dir = parent_dir;
    parent_dir->num_files++;
    for (int i = 0; i < parent_dir->num_files; i++) {
        if (!parent_dir->files[i]) {
            parent_dir->files[i] = head;
            if (DEBUG)
                printf("Inserted file %s into dir %s\n", parent_dir->files[i]->name, parent_dir->name);
            break;
        }
    }
    head->start = start_block;
    FatEntry* block = request_blocks(disk, 1);
    File* file = (File*) find_block(disk);
    block->file = file;
    file->block = block;
    file->free_in_block = BLOCK_SIZE - sizeof(File);
    file->data = (char*) file + sizeof(File);

    if (DEBUG) {
        printf("File %s created successfully\n", filename);
        printf("Name: %s\tParent directory: %s\tSize: %d\tStart: %p\n", head->name, head->parent_dir->name, head->size, head->start);
    }

    return 0;
}

Dir* create_dir(Disk* disk, Dir* parent_dir, char* dirname) {
    if (DEBUG)
        printf("Creating directory %s...\n", dirname);
    if (parent_dir && dir_exists(dirname, parent_dir)) {
        printf("Unable to create direcotry: directory already exists!\n");
        return NULL;
    }

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
    new_dir->num_dirs = 0;
    new_dir->dirs = (Dir**) (new_dir + sizeof(Dir) - sizeof(FileHead**));
    new_dir->files = (FileHead**) (new_dir + sizeof(Dir));

    if (parent_dir) {
        parent_dir->num_dirs++;
        for (int i = 0; i < parent_dir->num_dirs; i++) {
            if (!parent_dir->dirs[i]) {
                parent_dir->dirs[i] = new_dir;
                break;
            } 
        }
    }

    if (DEBUG) {
        printf("Directory %s created successfully\n", dirname);
        if (parent_dir)
            printf("Name: %s\t Parent dir: %s\tStart: %p\n", new_dir->name, new_dir->parent_dir->name, new_dir->start);
    }

    return new_dir;
}

int list_dir(Dir* dir) {
    printf("Content of %s:\n", dir->name);
    int i;
    int sum = 0;
    printf("Dir\tName\t\tFiles/Size\n");
    for (i = 0; i < dir->num_dirs; i++) {
        printf("%d\t%s\t\t%d\n", dir->dirs[i]->is_dir, dir->dirs[i]->name, dir->dirs[i]->num_files);
    }
    sum += i;
    for (i = 0; i < dir->num_files; i++) {
        printf("%d\t%s\t%d\n", dir->files[i]->is_dir, dir->files[i]->name, dir->files[i]->size);
    }
    sum += i;
    return sum;
}

int change_dir(char* dirname, Dir** cur_dir) {
    if (DEBUG)
        printf("Opening directory %s ...\n", dirname);
    if (!strcmp(dirname, "..")) {
        if (!(*cur_dir)->parent_dir) {
            printf("Unable to open directory: root has no parent directory!\n");
            return -1;
        }
        *cur_dir = (*cur_dir)->parent_dir;
        if (DEBUG)
            printf("Switched current directory to %s\n", (*cur_dir)->name);
        return 0;
    }
    if (!dir_exists(dirname, *cur_dir)) {
        printf("Unable to open directory: directory doesn't exist!\n");
        return -1;
    }
    for (int i = 0; i < (*cur_dir)->num_dirs; i++) {
        if (!strcmp(dirname, (*cur_dir)->dirs[i]->name)) {
            *cur_dir = (*cur_dir)->dirs[i];
            if (DEBUG)
                printf("Switched current directory to %s\n", (*cur_dir)->name);
            break;
        }
    }
    if (DEBUG)
        printf("Succesfully opened directory %s\n", dirname);
    return 0;
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
    if (DEBUG)
        printf("Reading file %s\n", filename);
    if (!file_exists(filename, cur_dir)) {
        printf("Unable to read file: file doesn't exist in the current directory!\n");
        return -1;
    }

    FileHead* head = open_file(filename, cur_dir, disk);
    if (!head)
        handle_error("error opening file!");
    
    printf("Content of %s:\n", filename);
    int n_bytes = head->size;
    int sum = 0;
    FatEntry* block = head->start;
    File* file;
    int next_idx;
    while (1) {
        // readite in head
        // while (i < n_bytes && i < BLOCK_SIZE - sizeof(FileHead)) {
        //     printf("%c", head->data[i]);
        //     i++;
        // }
        // read in other blocks
        int j = 0;
        if (sum == 0)
            file = block->file;
        else
            file = disk->fat->array[next_idx].file;
        // read until the block is full
        if (DEBUG)
            printf("Reading from block...\n");
        while (j < n_bytes && j < BLOCK_SIZE - sizeof(File)) {
            printf("%c", file->data[j]);
            j++;
        }
        sum += j;
        if (sum == n_bytes)
            break;
        // read from next block
        next_idx = block->data;
    }
    printf("\nEnd of content\n");
    return sum;
}

int write_file(char* filename, char* buf, int n_bytes, Dir* cur_dir, Disk* disk) {
    if (DEBUG) {
        printf("Writing file %s\n", filename);
        printf("Input is:\n");
        for (int i = 0; buf[i]; i++) {
            printf("%c", buf[i]);
        }
        printf("\n");
    }
    // if (!file_exists(filename, cur_dir)) {
    //     printf("Unable to write file: file doesn't exist in the current directory!\n");
    //     return -1;
    // }

    FileHead* head = open_file(filename, cur_dir, disk);
    // int i = 0;
    // // write until the block is full
    // while(head->start->file->free_in_block > 0 && i < n_bytes) {
    //     head->start->file->data[i] = buf[i];
    //     head->start->file->size++;
    //     head->start->file->free_in_block--;
    //     i++;
    // }
    // int allocated_blocks = 0;
    // FatEntry* prev_block;
    // while (i < n_bytes) {
    //     if (DEBUG)
    //         printf("Requesting new block...\n");
    //     FatEntry* block = request_blocks(disk, 1);
    //     prev_block = block;
    //     File* file = (File*) find_block(disk);
    //     file->block = block;
    //     file->free_in_block = BLOCK_SIZE - sizeof(File);
    //     // update the previous block to point to the new one
    //     if (DEBUG)
    //         printf("Updating FAT...\n");
    //     if (!allocated_blocks)
    //         head->start->data = block->idx;
    //     else {
    //         prev_block->data = block->idx;
    //     }
    //     allocated_blocks++;
    //     // for (int j = 0; j < FAT_BLOCKS_MAX; j++) {      // SUS, PROBABLY NEEDS FIXING
    //     //     // if (!(head->start + j + 1)) {
    //     //     //     head->start[j+1] = *block;
    //     //     //     break;
    //     //     // }
            
    //     // }
    //     // write until the block is full
    //     if (DEBUG)
    //         printf("Writing in new block...\n");
    //     while (file->free_in_block > 0 && i < n_bytes) {
    //         file->data[i] = buf[i];
    //         file->free_in_block--;
    //         head->size++;
    //         i++;
    //     }
    //  }
    int sum = 0;
    FatEntry* block = head->start;
    FatEntry* prev_block;
    File* file;
    int next_idx;
    while (1) {
        int j = 0;
        if (sum == 0)
            file = (File*) head->start->file;
        else
            file = disk->fat->array[next_idx].file;
        // write until the block is full
        if (DEBUG)
            printf("Writing in block...\n");
        printf("Free: %d\n", file->free_in_block);
        while (j < n_bytes && file->free_in_block > 0) {
            file->data[j] = buf[j];
            file->free_in_block--;
            head->size++;
            j++;
        }
        sum += j;
        printf("Sum is %d\n", sum);
        if (sum == n_bytes)
            break;
        // allocate new block
        if (DEBUG)
            printf("Requesting new block...\n");
        prev_block = block;
        block = request_blocks(disk, 1);
        prev_block->data = block->idx;
        next_idx = block->idx;
        file = (File*) find_block(disk);
        block->file = file;
        file->block = block;
        file->free_in_block = BLOCK_SIZE - sizeof(File);
    }
    return sum;
}