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

    // request FAT block for head
    FatEntry* start_block = request_fat_blocks(disk, NULL, 1);
    if (!start_block) {
        printf("Unable to create file: not enough FAT blocks available!\n");
        return -1;
    }

    // request disk block for head
    FileHead* head = (FileHead*) find_block(disk);
    if (!head)
        printf("Unable to create file: cannot find a block in the disk!\n");
    start_block->file = (File*) head;
    head->name = filename;
    head->is_dir = 0;
    head->size = 0;
    head->pos = 0;
    head->parent_dir = parent_dir;
    head->start = start_block;
    parent_dir->num_files++;
    for (int i = 0; i < parent_dir->num_files; i++) {
        if (!parent_dir->files[i]) {
            parent_dir->files[i] = head;
            if (DEBUG)
                printf("Inserted file %s into dir %s\n", parent_dir->files[i]->name, parent_dir->name);
            break;
        }
    }
    
    // request FAT block for data
    FatEntry* block = request_fat_blocks(disk, head->start, 1);
    // request disk block for data
    File* file = (File*) find_block(disk);
    //printf("File pointer at %p\n", file);
    file->block = block;
    file->free_in_block = BLOCK_SIZE - sizeof(File);
    //printf("free: %d\n", file->free_in_block);
    file->data = (char*) &file->data + sizeof(file->data);
    block->file = file;
    //printf("block->file: %p %p free: %d\n", block->file, *block->file, (*block->file)->free_in_block);

    if (DEBUG) {
        int idx = head->start->data;
        printf("File %s created successfully\n", filename);
        printf("Name: %s\tParent directory: %s\tSize: %d\tHead: %p\tData: %p\n", head->name, head->parent_dir->name, head->size, head->start->file, disk->fat->array[idx].file);
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
    FatEntry* start_block = request_fat_blocks(disk, NULL, 1);
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

int delete_file(char* filename, Dir* cur_dir, Disk* disk) {
    if (DEBUG)
        printf("Deleting file %s\n", filename);

    FileHead* head = open_file(filename, cur_dir, disk);
    if (!head) {
        printf("Unable to delete file: file doesn't exist in current directory!\n");
        return -1;
    }
    
    // clean the head
    int entry_idx = head->start->data;
    int next_entry_idx;
    //printf("Cleaning head FAT data...\n");
    head->start->busy = 0;
    head->start->data = -1;
    disk->fat->free_blocks++;
    //printf("Cleaning head disk data...\n");
    memset(head->start->file, 0, BLOCK_SIZE - sizeof(FileHead));

    // clean the other blocks
    FatEntry* cur_fat_block = &disk->fat->array[entry_idx];
    for (int i = 0; cur_fat_block; i++) {
        next_entry_idx = cur_fat_block->data;
        //printf("Cleaning block FAT data...\n");
        cur_fat_block->busy = 0;
        cur_fat_block->data = -1;
        disk->fat->free_blocks++;
        //printf("Cleaning block disk data...\n");
        memset(cur_fat_block->file, 0, BLOCK_SIZE - sizeof(File));
        if (next_entry_idx != -1)
            cur_fat_block = &disk->fat->array[next_entry_idx];
        else
            break;
    }
    cur_dir->num_files--;
    printf("File %s deleted succesfully\n", filename);
    return 0;
}

int delete_dir(char* dirname, Disk* disk, Dir* cur_dir) {
    if (!dir_exists(dirname, cur_dir)) {
        printf("Unable to delete directory: directory doesn't exist!\n");
        return -1;
    }

    Dir* to_delete;
    int idx;
    for (int i = 0; i < cur_dir->num_dirs; i++) {
        if (!strcmp(cur_dir->dirs[i]->name, dirname)) {
            idx = i;
            to_delete = cur_dir->dirs[i];
            break;
        }
    }
    int ret = delete_dir_aux(disk, to_delete, idx);
    if (ret)
        handle_error("Error in delete_dir_aux!");
    cur_dir->num_dirs--;
    printf("Directory %s deleted succesfully\n", dirname);
    return 0;
}

int delete_dir_aux(Disk* disk, Dir* dir, int idx) {
    int ret;
    Dir* next_dir = dir->dirs[idx+1];
    // we use a recursive call to delete the subdirectories
    if (next_dir) {
        ret = delete_dir_aux(disk, next_dir, idx);
        if (ret)
            handle_error("Error in delete_dir_aux!");
    }
    // we delete all files in the directory
    for (int i = 0; i < dir->num_files; i++) {
        FileHead* file = dir->files[i];
        ret = delete_file(file->name, dir, disk);
        if (ret)
            handle_error("Error in delete_file!");
    }
    // we clean the directory's FAT data
    dir->start->busy = 0;
    dir->start->data = -1;
    disk->fat->free_blocks++;
    // we clean the directory's disk data
    memset(dir, 0, BLOCK_SIZE - sizeof(Dir));
    return 0;
}

int list_dir(Dir* dir) {
    printf("Content of %s:\n", dir->name);
    int i;
    int sum = 0;
    printf("Dir\tName\t\tFiles/Size\tLocation:\n");
    for (i = 0; i < dir->num_dirs; i++) {
        printf("%d\t%s\t\t%d\t\t%p\n", dir->dirs[i]->is_dir, dir->dirs[i]->name, dir->dirs[i]->num_files, dir->dirs[i]);
    }
    sum += i;
    for (i = 0; i < dir->num_files; i++) {
        printf("%d\t%s\t%d\t\t%p\n", dir->files[i]->is_dir, dir->files[i]->name, dir->files[i]->size, dir->files[i]);
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

int read_file(char* filename, int pos, int n_bytes, Dir* cur_dir, Disk* disk) {
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
    if (!n_bytes)
        n_bytes = head->size;
    int sum = 0;
    FatEntry* block = head->start;
    File* file;
    int next_idx;
    while (1) {
        int j = 0;
        int idx;
        if (sum == 0) {
            idx = block->data;
            //printf("Reading from FAT %d\n", idx);
            file = disk->fat->array[idx].file;
        } else {
            idx = next_idx;
            //printf("Reading from FAT %d\n", idx);
            file = disk->fat->array[next_idx].file;
        }
        block = file->block;
        // read until the block is full
        if (DEBUG)
            printf("\nReading from block...\n");
        while (j < n_bytes && sum < n_bytes && j < BLOCK_SIZE - sizeof(File)) {
            printf("%c", file->data[j]);
            j++;
            sum++;
        }
        if (sum == n_bytes)
            break;
        // read from next block
        next_idx = block->data;
    }
    printf("\nEnd of content\n");
    return sum;
}

int write_file(char* filename, char* buf, int pos, int n_bytes, Dir* cur_dir, Disk* disk) {
    if (DEBUG) {
        printf("Writing file %s\n", filename);
        printf("Input is:\n");
        for (int i = 0; buf[i]; i++) {
            printf("%c", buf[i]);
        }
        printf("\n");
    }

    FileHead* head = open_file(filename, cur_dir, disk);
    if (!head)
        handle_error("Error opening file");
    //printf("Opened file with head %p\n", head);
    int sum = 0;    // sum of written bytes
    FatEntry* block = head->start;
    //printf("Head points to %p in the FAT\n", block);
    FatEntry* prev_block;   // to store the previous FAT block when allocating a new one
    File* file;     // pointer to the disk block
    int next_idx;   // to update the FAT after allocating a new block
    int buf_pos = 0;    // current position in the input buffer
    while (1) {
        int j = 0;
        int idx;
        if (sum == 0) {
            idx = block->data;
            //printf("Writing in FAT %d\n", idx);
            file = disk->fat->array[idx].file;
            //printf("First iteration: file points to %p in the disk (%d in FAT, next is %d)\n", file, head->start->idx, idx);
        } else {
            idx = next_idx;
            //printf("Writing in FAT %d\n", idx);
            file = disk->fat->array[idx].file;
            //printf("File points to %p in the disk (%d in FAT, next is %d)\n", file, head->start->idx, idx);
        }
        block = file->block;
        // write until the block is full
        if (DEBUG)
            printf("Writing in block (file: %s address: %p %p)...\n", head->name, file, file->data);
        //printf("Free bytes in block: %d\n", file->free_in_block);
        while (j < n_bytes && sum < n_bytes && file->free_in_block > 0) {
            file->data[j] = buf[buf_pos];
            printf("%c", file->data[j]);
            file->free_in_block--;
            head->size++;
            // printf("Written %d bytes\n", sum);
            // printf("Remaining bytes in block: %d\n", file->free_in_block);
            j++;
            buf_pos++;
            sum++;
        }
        printf("\n");
        //printf("Written %d bytes\n", sum);
        // if we are done we exit
        if (sum == n_bytes)
            break;
        // otherwise we allocate a new block
        if (DEBUG)
            printf("Requesting new block...\n");
        prev_block = block;
        block = request_fat_blocks(disk, prev_block, 1);
        prev_block->data = block->idx;
        next_idx = block->idx;
        file = (File*) find_block(disk);
        file->block = block;
        file->free_in_block = BLOCK_SIZE - sizeof(File);
        file->data = (char*) file + sizeof(File);
        block->file = file;
    }
    return sum;
}

char* seek_in_file(char* filename, int pos, Dir* cur_dir, Disk* disk) {
    if (pos < 0) {
        printf("Position not valid!\n");
        return NULL;
    }
    FileHead* file = open_file(filename, cur_dir, disk);
    if (!file)
        handle_error("Error opening file");
    file->pos = pos;

    FatEntry* block = file->start;
    int block_num = pos / (BLOCK_SIZE - sizeof(File));
    int block_offset = pos % (BLOCK_SIZE - sizeof(File));
    for (int i = 0; i < block_num; i++) {
        int next_idx = block->data;
        block = &disk->fat->array[next_idx];
    }
    printf("Block number: %d\nBlock offset: %d\n", block_num, block_offset);
    printf("Position in file %s: %d\n", filename, file->pos);
    return block->file->data + block_offset;
}