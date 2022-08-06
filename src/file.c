#include "headers/error.h"
#include "headers/file.h"
#include "headers/disk.h"
#include <stdio.h>
#include <string.h>

int directory_exists = 0;
int invalid_directory = 0;

FatEntry* get_fat_entry_ptr(int idx, Disk* disk) {
    FatEntry* entry_ptr;
    for (int i = 0; i < FAT_BLOCKS_MAX; i++) {
        if (idx == disk->fat.array[i].file) {;
            entry_ptr = &disk->fat.array[i];
            // if (DEBUG)
            //     printf("FAT entry pointer found at %p\n", &disk->fat.array[i]);
            return entry_ptr;
        }
    }
    return NULL;
}

FileHead* get_file_head_ptr(int idx, Disk* disk) {
    FileHead* file_ptr;
    for (int i = 0; i < disk->size; i += BLOCK_SIZE) {
        int cur_idx = (int) disk->data[i];
        if (cur_idx == idx) {
            file_ptr = (FileHead*) &disk->data[i];
            // if (DEBUG)
            //     printf("File head pointer found at %p\n", file_ptr);
            return file_ptr;
        }
    }
    return NULL;
}

File* get_file_ptr(int idx, Disk* disk) {
    File* file_ptr;
    for (int i = 0; i < disk->size; i += BLOCK_SIZE) {
        int cur_idx = (int) disk->data[i];
        if (cur_idx == idx) {
            file_ptr = (File*) &disk->data[i];
            // if (DEBUG)
            //     printf("File pointer found at %p\n", file_ptr);
            return file_ptr;
        }
    }
    return NULL;
}

Dir* get_dir_ptr(int idx, Disk* disk) {
    Dir* cur_dir_ptr;
    for (int i = 0; i < disk->size; i += BLOCK_SIZE) {
        int cur_idx = (int) disk->data[i];
        if (cur_idx == idx) {
            cur_dir_ptr = (Dir*) &disk->data[i];
            // if (DEBUG)
            //     printf("Dir pointer found at %p\n", cur_dir_ptr);
            return cur_dir_ptr;
        }
    }
    return NULL;
}

int file_exists(char* filename, int cur_dir, Disk* disk) {
    if (DEBUG)
        printf("Checking existance of file %s...\n", filename);
    char filenamecpy[MAX_PATH_LENGTH];
    char filenamecpy2[30];
    strncpy(filenamecpy, filename, strlen(filename) + 1);
    strncpy(filenamecpy2, filename, strlen(filename) + 1);
    //printf("filenamecpy: %s\n", filenamecpy);
    char path[MAX_PATH_LENGTH];
    char prevtoken[30];
    int token_count = 0;
    char* token = strtok(filenamecpy, "/");
    if (token)
        token_count++;
    memset(path, 0, strlen(path) + 1);
    //printf("path: %s\n", path);
    //printf("token: %s\n", token);
    while(token) {
        strncpy(prevtoken, token, strlen(token) + 1);
        //printf("prevtoken: %s\n", prevtoken);
        token = strtok(NULL, "/");
        if (token) {
            token_count++;
            if (token_count > 1 && strncmp(filename, "..", 2))
                strncat(path, "/", 2);
            strncat(path, prevtoken, strlen(prevtoken) + 1);
            //printf("path: %s\n", path);
        }
        //printf("token: %s\n", token);
    }
    if (token_count > 1 || !strncmp(filename, "/", 1)) {
        if (!path[0])
            path[0] ='/';
        //printf("Path: %s\nFilename: %s\n", path, prevtoken);
        int dir_for_file;
        if (!dir_exists(path, cur_dir, disk, &dir_for_file)) {
            //printf("Error: directory %s doesn't exist!\n", path);
            directory_exists = 1;
            return -1;
        }
        //printf("DIR FOR FILE: %s\n", get_dir_ptr(dir_for_file, disk)->name);
        memset(filenamecpy2, 0, strlen(filenamecpy2) + 1);
        strncpy(filenamecpy2, prevtoken, strlen(prevtoken) + 1);
        // Needs to change cur_dir to the correct directory
        //Dir* t_ptr = get_dir_ptr(cur_dir, disk);
        //printf("Cur dir before dir_for_file is %s\n", t_ptr->name);
        //printf("Files: %d\nDirs: %d\n", t_ptr->num_files, t_ptr->num_dirs);
        if (dir_for_file) {
            cur_dir = dir_for_file;
            if (DEBUG)
                printf("Cur dir changed to %s (dir_for_file)\n", get_dir_ptr(cur_dir, disk)->name);
        }
    }

    Dir* cur_dir_ptr = get_dir_ptr(cur_dir, disk);
    //printf("Cur dir: %s\n", cur_dir_ptr->name);
    //printf("Files: %d\nDirs: %d\n", cur_dir_ptr->num_files, cur_dir_ptr->num_dirs);
    for (int i = 0; i < cur_dir_ptr->num_files + cur_dir_ptr->num_dirs; i++) {
        FileHead* file_ptr = get_file_head_ptr(cur_dir_ptr->files[i], disk);
        //printf("Comparing %s : %s\n", file_ptr->name, filenamecpy2);
        if (!strncmp(file_ptr->name, filenamecpy2, 30) && !file_ptr->is_dir) {
            if (DEBUG)
                printf("File exists\n");
            return 1;
        }
    }
    return 0;
}

int dir_exists(char* dirname, int cur_dir, Disk* disk, int* dir_for_file) {
    if (DEBUG)
        printf("Checking existance of directory %s ...\n", dirname);
    if (!strncmp(dirname, "", MAX_PATH_LENGTH) || !strncmp(dirname, "/", MAX_PATH_LENGTH)) {
        if (dir_for_file)
            *dir_for_file = disk->root_dir;
        if (DEBUG)
            printf("Dir %s exists\n", dirname);
        return 1;
    }
    if (!strncmp(dirname, "/", 1))
        cur_dir = disk->root_dir;
    //printf("Cur dir is %s\n", get_dir_ptr(cur_dir, disk)->name);
    char dirnamecpy[MAX_PATH_LENGTH];
    char prevtoken[30];
    strncpy(dirnamecpy, dirname, strlen(dirname) + 1);
    char* token = strtok(dirnamecpy, "/");
    //printf("token is %s\n", token);
    while (token) {
        strncpy(prevtoken, token, strlen(token) + 1);
        int ret = dir_exists_aux(token, &cur_dir, disk);
        //printf("Dir: %s\n", get_dir_ptr(cur_dir, disk)->name);
        if (dir_for_file) {
            *dir_for_file = cur_dir;
            //printf("dir_for_file is now %s\n", get_dir_ptr(*dir_for_file, disk)->name);
        }
        token = strtok(NULL, "/");
        if (!ret) {
            if (!token) {
                if (DEBUG)
                    printf("Directory does not exist!\n");
                return 0;
            } else {
                if (!directory_exists)
                    printf("Unable to open directory: directory %s doesn't exist!\n", prevtoken);
                invalid_directory = 1;
                return 0;
            }
        }
        //printf("token is %s\n", token);
    }
    return 1;
}

int dir_exists_aux(char* dirname, int* cur_dir, Disk* disk) {
    if (DEBUG)
        printf("Checking existance of directory %s (aux)...\n", dirname);
    Dir* cur_dir_ptr = get_dir_ptr(*cur_dir, disk);
    if (!strncmp(dirname, "..", MAX_PATH_LENGTH) && !cur_dir_ptr->parent_dir) {
        printf("Unable to open directory: root has no parent directory!\n");
        return 0;
    }
    //printf("cur dir is %s\n", cur_dir_ptr->name);
    if (!strncmp(dirname, "..", MAX_PATH_LENGTH)) {
        Dir* par_dir_ptr = get_dir_ptr(cur_dir_ptr->parent_dir, disk);
        //printf("%s exists\n", par_dir_ptr->name);
        *cur_dir = par_dir_ptr->idx;
        return 1;
    }
    for (int i = 0; i < cur_dir_ptr->num_dirs + cur_dir_ptr->num_files; i++) {
        Dir* subdir_ptr = get_dir_ptr(cur_dir_ptr->files[i], disk);
        if (!strncmp(subdir_ptr->name, dirname, MAX_PATH_LENGTH) && subdir_ptr->is_dir) {
            if (DEBUG)
                printf("Directory %s exists\n", subdir_ptr->name);
            *cur_dir = subdir_ptr->idx;
            return 1;
        }
    }
    return 0;
}

int create_file(char* filename, int parent_dir, Disk* disk) {
    // 1) Check if file already exists
    // 2) Request free block(s)
    // 3) Allocate block(s)

    if (DEBUG)
        printf("Creating file %s...\n", filename);

    char filenamecpy[MAX_PATH_LENGTH];
    strncpy(filenamecpy, filename, strlen(filename) + 1);

    int ret = file_exists(filename, parent_dir, disk);
    if (ret) {
        if (ret == -1)
            printf("Unable to create file: directory does not exist!\n");
        else
            printf("Unable to create file: file already exists!\n");
        return -1;
    }

    //printf("filenamecpy: %s\n", filenamecpy);
    char path[MAX_PATH_LENGTH];
    char prevtoken[30];
    int token_count = 0;
    char* token = strtok(filenamecpy, "/");
    if (token)
        token_count++;
    memset(path, 0, strlen(path) + 1);
    //printf("path: %s\n", path);
    //printf("token: %s\n", token);
    while(token) {
        strncpy(prevtoken, token, strlen(token) + 1);
        //printf("prevtoken: %s\n", prevtoken);
        token = strtok(NULL, "/");
        if (token) {
            token_count++;
            if (token_count > 1 && strncmp(filename, "..", 2))
                strncat(path, "/", 2);
            strncat(path, prevtoken, strlen(prevtoken) + 1);
            //printf("path: %s\n", path);
        }
        //printf("token: %s\n", token);
    }
    if (token_count > 1 || !strncmp(filename, "/", 1)) {
        //  printf("Path: %s\nFilename: %s\n", path, prevtoken);
        change_dir(path, &disk->cur_dir, disk);
        parent_dir = disk->cur_dir;
        memset(filename, 0, strlen(filename) + 1);
        strncpy(filename, prevtoken, strlen(prevtoken) + 1);
    }

    if (strlen(filename) > 30) {
        printf("Unable to create file: file name length cannot exceed 30 characters!\n");
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
    head->idx = get_block_idx(disk);
    start_block->file = head->idx;
    for (int i = 0; filename[i]; i++)
        head->name[i] = filename[i];
    head->is_dir = 0;
    head->size = 0;
    head->pos = 0;
    head->parent_dir = parent_dir;
    head->start = get_fat_entry_idx(disk, start_block);
    if (head->start == -1) {
        printf("Error getting FAT entry index!\n");
        return -1;
    }
    Dir* parent_dir_ptr = get_dir_ptr(parent_dir, disk);
    parent_dir_ptr->num_files++;
    for (int i = 0; i < parent_dir_ptr->num_files + parent_dir_ptr->num_dirs; i++) {
        if (!parent_dir_ptr->files[i]) {
            parent_dir_ptr->files[i] = head->idx;
            if (DEBUG)
                printf("Inserted file %s into dir %s\n", head->name, parent_dir_ptr->name);
            break;
        }
    }
    
    // request FAT block for data
    FatEntry* entry_ptr = get_fat_entry_ptr(head->idx, disk);
    FatEntry* block = request_fat_blocks(disk, entry_ptr, 1);
    // request disk block for data
    File* file = (File*) find_block(disk);
    //printf("File pointer at %p\n", file);
    file->idx = get_block_idx(disk);
    file->free_in_block = BLOCK_SIZE;
    //printf("free: %d\n", file->free_in_block);
    block->file = file->idx;
    //printf("block->file: %p %p free: %d\n", block->file, *block->file, (*block->file)->free_in_block);

    printf("File %s created successfully\n", filename);
    if (DEBUG)
        printf("Name: %s\tParent directory: %s\tSize: %d\tIndex: %d\tFAT idx: %d\n", head->name, parent_dir_ptr->name, head->size, head->idx, head->start);

    return 0;
}

Dir* create_dir(char* dirname, int parent_dir, Disk* disk) {
    if (DEBUG)
        printf("Creating directory %s...\n", dirname);
    
    if (parent_dir && dir_exists(dirname, parent_dir, disk, NULL)) {
        if (!invalid_directory)
            printf("Unable to create directory: directory already exists!\n");
        return NULL;
    }
    if (invalid_directory)
        return NULL;

    if (parent_dir) {
        char dirnamecpy[MAX_PATH_LENGTH];
        strncpy(dirnamecpy, dirname, strlen(dirname) + 1);
        char path[MAX_PATH_LENGTH];
        char prevtoken[30];
        int token_count = 0;
        char* token = strtok(dirnamecpy, "/");
        memset(path, 0, strlen(path) + 1);
        if (token)
            token_count++;
        //printf("path: %s\n", path);
        //printf("token: %s\n", token);
        while(token) {
            strncpy(prevtoken, token, strlen(token) + 1);
            //printf("prevtoken: %s\n", prevtoken);
            token = strtok(NULL, "/");
            if (token) {
                token_count++;
                if (token_count > 1 && strncmp(dirname, "..", 2))
                    strncat(path, "/", 2);
                strncat(path, prevtoken, strlen(prevtoken) + 1);
                //printf("path: %s\n", path);
            }
            //printf("token: %s\n", token);
        }
        if (token_count > 1 || !strncmp(dirname, "/", 1)) {
            //printf("Path: %s\nFilename: %s\n", path, prevtoken);
            change_dir(path, &disk->cur_dir, disk);
            parent_dir = disk->cur_dir;
            memset(dirname, 0, strlen(dirname) + 1);
            strncpy(dirname, prevtoken, strlen(prevtoken) + 1);
        }

        if (strlen(dirname) > 30) {
            printf("Unable to create directory: directory name length cannot exceed 30 characters!\n");
            return NULL;
        }
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

    new_dir->idx = get_block_idx(disk);
    if (!new_dir->idx) {
        printf("Error getting block index!\n");
        return NULL;
    }
    new_dir->is_dir = 1;
    for (int i = 0; dirname[i]; i++)
        new_dir->name[i] = dirname[i];
    new_dir->parent_dir = parent_dir;
    new_dir->num_files = 0;
    new_dir->num_dirs = 0;
    start_block->file = new_dir->idx;

    Dir* parent_dir_ptr;
    if (parent_dir) {
        parent_dir_ptr = get_dir_ptr(parent_dir, disk);
        parent_dir_ptr->num_dirs++;
        for (int i = 0; i < parent_dir_ptr->num_dirs + parent_dir_ptr->num_files; i++) {
            if (!parent_dir_ptr->files[i]) {
                parent_dir_ptr->files[i] = new_dir->idx;
                break;
            } 
        }
    }

    if (parent_dir)
        printf("Directory %s created successfully\n", dirname);
    if (DEBUG) {
        if (parent_dir)
            printf("Name: %s\t Parent dir: %s\tFiles: %d\tIndex: %d\n", new_dir->name, parent_dir_ptr->name, new_dir->num_files, new_dir->idx);
        else
            printf("Name: %s\t Parent dir: -\tFiles: %d\tIndex: %d\n", new_dir->name, new_dir->num_files, new_dir->idx);
    }

    return new_dir;
}

int delete_file(char* filename, int cur_dir, int sub, Disk* disk) {
    if (DEBUG)
        printf("Deleting file %s\n", filename);

    char filenamecpy[MAX_PATH_LENGTH];
    strncpy(filenamecpy, filename, strlen(filename) + 1);

    if (!file_exists(filename, cur_dir, disk)) {
        printf("Unable to delete file: file doesn't exist!\n");
        return -1;
    }

    //printf("filenamecpy: %s\n", filenamecpy);
    char path[MAX_PATH_LENGTH];
    char prevtoken[30];
    int token_count = 0;
    char* token = strtok(filenamecpy, "/");
    if (token)
        token_count++;
    memset(path, 0, strlen(path) + 1);
    //printf("path: %s\n", path);
    //printf("token: %s\n", token);
    while(token) {
        strncpy(prevtoken, token, strlen(token) + 1);
        //printf("prevtoken: %s\n", prevtoken);
        token = strtok(NULL, "/");
        if (token) {
            token_count++;
            if (token_count > 1 && strncmp(filename, "..", 2))
                strncat(path, "/", 2);
            strncat(path, prevtoken, strlen(prevtoken) + 1);
            //printf("path: %s\n", path);
        }
        //printf("token: %s\n", token);
    }
    if (token_count > 1 || !strncmp(filename, "/", 1)) {
        //printf("Path: %s\nFilename: %s\n", path, prevtoken);
        change_dir(path, &disk->cur_dir, disk);
        cur_dir = disk->cur_dir;
        memset(filename, 0, strlen(filename) + 1);
        strncpy(filename, prevtoken, strlen(prevtoken) + 1);
    }


    FileHead* head = open_file(filename, cur_dir, disk);
    if (!head) {
        if (!directory_exists)
            printf("Unable to delete file: file doesn't exist in current directory!\n");
        return -1;
    }

    // properly set the pointers to the other files to fill the hole
    Dir* cur_dir_ptr = get_dir_ptr(cur_dir, disk);
    for (int i = 0; i < cur_dir_ptr->num_files + cur_dir_ptr->num_dirs; i++) {
        FileHead* file = get_file_head_ptr(cur_dir_ptr->files[i], disk);
        if (!strncmp(file->name, filename, 30) && !file->is_dir) {
            // printf("Found file\n");
            cur_dir_ptr->files[i] = 0;
        } else
            continue;
        while (!cur_dir_ptr->files[i] && i < cur_dir_ptr->num_files + cur_dir_ptr->num_dirs - 1) {
            // printf("Shifting file pointer...\n");
            cur_dir_ptr->files[i] = cur_dir_ptr->files[i+1];
            cur_dir_ptr->files[i+1] = 0;
            i++;
        }
    }
    
    FatEntry* entry = get_fat_entry_ptr(head->idx, disk);
    // clean the head
    int entry_idx = entry->data;
    int next_entry_idx;
    //printf("Cleaning head FAT data...\n");
    entry->file = 0;
    //entry->busy = 0;
    entry->data = -2;
    disk->fat.free_blocks++;
    //printf("Cleaning head disk data...\n");
    memset(head, 0, sizeof(FileHead));

    // clean the other blocks
    FatEntry* cur_fat_block = &disk->fat.array[entry_idx];
    if (!cur_fat_block) {
        printf("Error: cannot retreive FAT block pointer\n");
        return -1;
    }
    for (int i = 0; cur_fat_block; i++) {
        next_entry_idx = cur_fat_block->data;
        if (DEBUG) printf("Cleaning block FAT data...\n");
        //cur_fat_block->busy = 0;
        cur_fat_block->data = -2;
        disk->fat.free_blocks++;
        if (DEBUG) printf("Cleaning block disk data...\n");
        // printf("Getting file pointer for %d\n", cur_fat_block->file);
        File* file_ptr = get_file_ptr(cur_fat_block->file, disk);
        if (!file_ptr) {
            printf("Error: cannot retreive file pointer\n");
            return -1;
        }
        cur_fat_block->file = 0;
        memset(file_ptr, 0, sizeof(File));
        if (next_entry_idx != -1)
            cur_fat_block = &disk->fat.array[next_entry_idx];
        else
            break;
    }
    cur_dir_ptr->num_files--;
    if (!sub || DEBUG)
        printf("File %s deleted succesfully\n", filename);
    return 0;
}

int delete_dir(char* dirname, int cur_dir, Disk* disk) {
    if (!dir_exists(dirname, cur_dir, disk, NULL)) {
        if (!invalid_directory)
            printf("Unable to delete directory: directory doesn't exist!\n");
        return -1;
    }

    char dirnamecpy[MAX_PATH_LENGTH];
    strncpy(dirnamecpy, dirname, strlen(dirname) + 1);
    char path[MAX_PATH_LENGTH];
    char prevtoken[30];
    int token_count = 0;
    char* token = strtok(dirnamecpy, "/");
    if (token)
        token_count++;
    memset(path, 0, strlen(path) + 1);
    //printf("path: %s\n", path);
    //printf("token: %s\n", token);
    while(token) {
        strncpy(prevtoken, token, strlen(token) + 1);
        //printf("prevtoken: %s\n", prevtoken);
        token = strtok(NULL, "/");
        if (token) {
            token_count++;
            if (token_count > 1 && strncmp(dirname, "..", 2))
                strncat(path, "/", 2);
            strncat(path, prevtoken, strlen(prevtoken) + 1);
            //printf("path: %s\n", path);
        }
        //printf("token: %s\n", token);
    }
    if (token_count > 1 || !strncmp(dirname, "/", 1)) {
        //printf("Path: %s\nFilename: %s\n", path, prevtoken);
        change_dir(path, &disk->cur_dir, disk);
        cur_dir = disk->cur_dir;
        memset(dirname, 0, strlen(dirname) + 1);
        strncpy(dirname, prevtoken, strlen(prevtoken) + 1);
    }

    Dir* to_delete;
    Dir* cur_dir_ptr = get_dir_ptr(cur_dir, disk);
    if (!cur_dir_ptr) {
        printf("Error getting current dir pointer\n");
        return -1;
    }
    for (int i = 0; i < cur_dir_ptr->num_dirs + cur_dir_ptr->num_files; i++) {
        Dir* sub_dir_ptr = get_dir_ptr(cur_dir_ptr->files[i], disk);
        if (!sub_dir_ptr) {
            printf("Error getting subdir pointer\n");
            return -1;
        }
        if (sub_dir_ptr->is_dir && !strncmp(sub_dir_ptr->name, dirname, 30)){
            to_delete = sub_dir_ptr;
            // printf("To delete: %s\n", to_delete->name);
            break;
        }
    }
    int ret = delete_dir_aux(disk, cur_dir_ptr, to_delete);
    if (ret)
        handle_error("Error in delete_dir_aux!");
    cur_dir_ptr->num_dirs--;
    printf("Directory %s deleted succesfully\n", dirname);
    return 0;
}

int delete_dir_aux(Disk* disk, Dir* cur_dir, Dir* dir) {
    int ret;
    if (dir->num_dirs) {
        int i = 0;
        int deleted = 0;
        int tot_dirs = dir->num_dirs;
        while (deleted < tot_dirs && tot_dirs + dir->num_files) {
            Dir* next_dir = get_dir_ptr(dir->files[i], disk);
            // we use a recursive call to delete the subdirectories
            if (next_dir && next_dir->is_dir) {
                ret = delete_dir_aux(disk, dir, next_dir);
                if (ret)
                    handle_error("Error in delete_dir_aux!");
                deleted++;
                // printf("Subdirectory %s deleted successfully\n", dir->name);
            } else
                i++;
        }
    }
    if (DEBUG)
        printf("Deleting directory %s\n", dir->name);
    // we delete all files in the directory
    int i = 0;
    int deleted = 0;
    int tot_files = dir->num_files;
    while (deleted < tot_files && i < dir->num_files + dir->num_dirs) {
        FileHead* file = get_file_head_ptr(dir->files[i], disk);
        // int file_idx = file->idx;
        if (file && !file->is_dir) {
            ret = delete_file(file->name, dir->idx, 1, disk);
            deleted++;
            if (ret)
                handle_error("Error in delete_file!");
        } else
            i++;
    }

    // properly set the pointers to the other dirs and files to fill the hole
    for (int i = 0; i < cur_dir->num_dirs + cur_dir->num_files; i++) {
        if (cur_dir->files[i] == dir->idx) {
            // printf("Found dir\n");
            cur_dir->files[i] = 0;
        } else
            continue;
        while (!cur_dir->files[i] && i < cur_dir->num_dirs + cur_dir->num_files - 1) {
            // printf("Shifting dir pointer...\n");
            cur_dir->files[i] = cur_dir->files[i+1];
            cur_dir->files[i+1] = 0;
            i++;
        }
    }

    // we clean the directory's FAT data
    // printf("Getting FAT entry pointer for dir idx: %d\n", dir->idx);
    FatEntry* entry = get_fat_entry_ptr(dir->idx, disk);      // < ---
    if (!entry) {
        printf("Error getting FAT block idx!\n");
        return -1;
    }
    entry->file = 0;
    //entry->busy = 0;
    entry->data = -2;
    disk->fat.free_blocks++;
    // we clean the directory's disk data
    memset(dir, 0, BLOCK_SIZE);
    return 0;
}

int list_dir(int dir, Disk* disk) {
    Dir* dir_ptr = get_dir_ptr(dir, disk);
    if (!dir_ptr) {
            printf("Error getting dir pointer\n");
            return -1;
        }
    printf("Content of %s:\n", dir_ptr->name);
    int i;
    int sum = 0;
    printf("Dir\tName\t\t\t\t\tDirs\tFiles\tSize\tCur pos\t\tBlock index\n");
    for (i = 0; i < dir_ptr->num_dirs + dir_ptr->num_files; i++) {
        // printf("Getting subdir pointer to %d\n", dir_ptr->files[i]);
        Dir* subdir_ptr = get_dir_ptr(dir_ptr->files[i], disk);
        if (!subdir_ptr) {
            printf("Error getting subdir pointer\n");
            return -1;
        }
        if (subdir_ptr->is_dir) {
            printf("%d\t%s\t\t\t\t\t%d\t%d\t-\t-\t\t%d\n", subdir_ptr->is_dir, subdir_ptr->name, subdir_ptr->num_dirs, subdir_ptr->num_files, subdir_ptr->idx);
            sum++;
        }
    }
    for (i = 0; i < dir_ptr->num_files + dir_ptr->num_dirs; i++) {
        // printf("Getting file head pointer to %d\n", dir_ptr->files[i]);
        FileHead* file_ptr = get_file_head_ptr(dir_ptr->files[i], disk);
        if (!file_ptr) {
            printf("Error getting file head pointer\n");
            return -1;
        }
        if (!file_ptr->is_dir) {
            printf("%d\t%s\t\t\t\t\t-\t-\t%d\t%d\t\t%d\n", file_ptr->is_dir, file_ptr->name, file_ptr->size, file_ptr->pos, file_ptr->idx);
            sum++;
        }
    }
    return sum;
}

int change_dir(char* dirname, int* cur_dir, Disk* disk) {
    //printf("dirname: %s\n", dirname);
    if (!strncmp(dirname, "", MAX_PATH_LENGTH) || !strncmp(dirname, "/", MAX_PATH_LENGTH)) {
        memset(disk->cur_path, 0, MAX_PATH_LENGTH);
        disk->cur_path[0] = '/';
        *cur_dir = disk->root_dir;
        if (DEBUG)
            printf("Switched current directory to %s\n", disk->cur_path);
        return 0;
    }
    if (!dir_exists(dirname, *cur_dir, disk, NULL)) {
        printf("Unable to open directory: directory doesn't exist!\n");
        return -1;
    }
    if (!strncmp(dirname, "/", 1)) {
        memset(disk->cur_path, 0, MAX_PATH_LENGTH);
        disk->cur_path[0] = '/';
        *cur_dir = disk->root_dir;
        if (DEBUG)
            printf("Switched current directory to %s\n", disk->cur_path);
    }
    char dirnamecpy[MAX_PATH_LENGTH];
    strncpy(dirnamecpy, dirname, strlen(dirname) + 1);
    char* token = strtok(dirnamecpy, "/");
    //printf("token is %s\n", token);
    while (token) {
        int ret = change_dir_aux(token, cur_dir, disk);
        if (DEBUG && ret) {
            printf("Error changing directory!\n");
            return -1;
        }
        token = strtok(NULL, "/");
        //printf("token is %s\n", token);
    }
    return 0;
}

int change_dir_aux(char* dirname, int* cur_dir, Disk* disk) {
    if (DEBUG)
        printf("Opening directory %s ...\n", dirname);
    Dir* cur_dir_ptr = get_dir_ptr(*cur_dir, disk);
    if (!strncmp(dirname, "..", MAX_PATH_LENGTH)) {
        Dir* parent_dir_ptr = get_dir_ptr(cur_dir_ptr->parent_dir, disk);
        if (!cur_dir_ptr->parent_dir) {
            printf("Unable to open directory: root has no parent directory!\n");
            return -1;
        }
        int j = 0;
        for (int i = strlen(disk->cur_path)-1; i >= 0; i--) {
            // printf("%c\n", disk->cur_path[i]);
            if (disk->cur_path[i] == '/') {
                // printf("SUB\n");
                if (i == 0)
                    memset(&disk->cur_path[i+1], 0, j);
                else
                    memset(&disk->cur_path[i], 0, j);
                break;
            }
            j++;
        }
        *cur_dir = parent_dir_ptr->idx;
        if (DEBUG)
            printf("Switched current directory to %s\n", parent_dir_ptr->name);
        return 0;
    }
    for (int i = 0; i < cur_dir_ptr->num_dirs + cur_dir_ptr->num_files; i++) {
        Dir* subdir_ptr = get_dir_ptr(cur_dir_ptr->files[i], disk);
        if (!strncmp(dirname, subdir_ptr->name, 30) && subdir_ptr->is_dir) {    
            if (strncmp(cur_dir_ptr->name, "/", 30))
                strncat(disk->cur_path, "/", 2);
            *cur_dir = subdir_ptr->idx;
            cur_dir_ptr = get_dir_ptr(*cur_dir, disk);
            strncat(disk->cur_path, cur_dir_ptr->name, strlen(cur_dir_ptr->name));
            if (DEBUG)
                printf("Switched current directory to %s\n", cur_dir_ptr->name);
            break;
        }
    }
    if (DEBUG)
        printf("Succesfully opened directory %s\n", dirname);
    return 0;
}

FileHead* open_file(char* filename, int cur_dir, Disk* disk) {
    // 1) Look for the file in the FAT
    // 2) Return the FileHead

    if (DEBUG)
        printf("Opening file %s\n", filename);
    FileHead* head;
    char* name;
    Dir* cur_dir_ptr = get_dir_ptr(cur_dir, disk);
    //printf("Dir is %s\n", cur_dir_ptr->name);
    for (int i = 0; i < cur_dir_ptr->num_files + cur_dir_ptr->num_dirs; i++) {
        head = get_file_head_ptr(cur_dir_ptr->files[i], disk);
        if (head->is_dir) {
            //printf("%s is dir\n", head->name);
            continue;
        }
        name = head->name;
        //printf("name: %s\n",name);
        if (!strncmp(filename, name, 30)) {
            return head;
        }
    }
    if (!directory_exists)
        printf("File not present in the current_directory!\n");
    return NULL;
}

int read_file(char* filename, int pos, int n_bytes, int cur_dir, Disk* disk) {
    if (DEBUG)
        printf("Reading file %s\n", filename);

    if (n_bytes < 0) {
        printf("Error: number of bytes is invalid!\n");
        return -1;
    }
    
    char filenamecpy[MAX_PATH_LENGTH];
    strncpy(filenamecpy, filename, strlen(filename) + 1);

    // Dir* cur_dir_ptr = get_dir_ptr(cur_dir, disk);
    if (!file_exists(filename, cur_dir, disk)) {
        printf("Unable to read file: %s doesn't exist in the current directory!\n", filename);
        return -1;
    }

    //printf("filenamecpy: %s\n", filenamecpy);
    char path[MAX_PATH_LENGTH];
    char prevtoken[30];
    int token_count = 0;
    char* token = strtok(filenamecpy, "/");
    if (token)
        token_count++;
    memset(path, 0, strlen(path) + 1);
    //printf("path: %s\n", path);
    //printf("token: %s\n", token);
    while(token) {
        strncpy(prevtoken, token, strlen(token) + 1);
        //printf("prevtoken: %s\n", prevtoken);
        token = strtok(NULL, "/");
        if (token) {
            token_count++;
            if (token_count > 1 && strncmp(filename, "..", 2))
                strncat(path, "/", 2);
            strncat(path, prevtoken, strlen(prevtoken) + 1);
            //printf("path: %s\n", path);
        }
        //printf("token: %s\n", token);
    }
    if (token_count > 1 || !strncmp(filename, "/", 1)) {
        //  printf("Path: %s\nFilename: %s\n", path, prevtoken);
        change_dir(path, &disk->cur_dir, disk);
        cur_dir = disk->cur_dir;
        memset(filename, 0, strlen(filename) + 1);
        strncpy(filename, prevtoken, strlen(prevtoken) + 1);
    }

    FileHead* head = open_file(filename, cur_dir, disk);
    if (!head)
        handle_error("error opening file!");
    if (pos >= head->size || pos < -2) {
        printf("Error: position is invalid!\n");
        return -1;
    }

    if(head->pos == -1)
        head->pos = head->size;

    printf("Content of %s:\n", filename);
    int sum = 0;
    FatEntry* block = get_fat_entry_ptr(head->idx, disk);
    File* file;
    int next_idx;

    if (pos == -1)
        pos = head->size;
    if (pos == -2)
        pos = head->pos;
    int block_num = pos / (BLOCK_SIZE - 2*sizeof(int));
    int block_offset = pos % (BLOCK_SIZE - 2*sizeof(int));
    if (!n_bytes)
        n_bytes = head->size - pos;

    while (1) {
        int j = 0;
        int idx;
        if (sum == 0) {
            for (int i = 0; i < block_num; i++) {
                idx = block->data;
                block = &disk->fat.array[idx];
            }
            idx = block->data;
            // printf("Reading from FAT %d\n", idx);
            // printf("Getting file ptr for FAT block %d\n", disk->fat.array[idx].file);
            file = get_file_ptr(disk->fat.array[idx].file, disk);
        } else {
            idx = next_idx;
            // printf("Reading from FAT %d\n", idx);
            // printf("Getting file ptr for FAT block %d\n", disk->fat.array[idx].file);
            file = get_file_ptr(disk->fat.array[next_idx].file, disk);
        }
        FatEntry* entry_ptr = get_fat_entry_ptr(file->idx, disk);
        block = entry_ptr;
        // read until the block is full
        if (DEBUG)
            printf("\nReading from block...\n");
        while (sum < n_bytes && j + block_offset < BLOCK_SIZE - 2*sizeof(int)) {
            printf("%c", file->data[j + block_offset]);
            j++;
            sum++;
        }
        block_offset = 0;
        if (sum == n_bytes)
            break;
        // read from next block
        next_idx = block->data;
    }
    printf("\n");
    // printf("\nEnd of content\n");
    return sum;
}

int write_file(char* filename, char* buf, int pos, int n_bytes, int cur_dir, Disk* disk) {
    if (DEBUG) {
        printf("Writing file %s\n", filename);
        printf("Input is:\n");
        for (int i = 0; buf[i]; i++) {
            printf("%c", buf[i]);
        }
        printf("\n");
    }
    if (n_bytes < 0) {
        printf("Error: number of bytes is invalid!\n");
        return -1;
    }

    char filenamecpy[MAX_PATH_LENGTH];
    strncpy(filenamecpy, filename, strlen(filename) + 1);
    
    if (!file_exists(filename, cur_dir, disk)) {
        printf("Unable to write file: %s doesn't exist in the current directory!\n", filename);
        return -1;
    }

    //printf("filenamecpy: %s\n", filenamecpy);
    char path[MAX_PATH_LENGTH];
    char prevtoken[30];
    int token_count = 0;
    char* token = strtok(filenamecpy, "/");
    if (token)
        token_count++;
    memset(path, 0, strlen(path) + 1);
    //printf("path: %s\n", path);
    //printf("token: %s\n", token);
    while(token) {
        strncpy(prevtoken, token, strlen(token) + 1);
        //printf("prevtoken: %s\n", prevtoken);
        token = strtok(NULL, "/");
        if (token) {
            token_count++;
            if (token_count > 1 && strncmp(filename, "..", 2))
                strncat(path, "/", 2);
            strncat(path, prevtoken, strlen(prevtoken) + 1);
            //printf("path: %s\n", path);
        }
        //printf("token: %s\n", token);
    }
    if (token_count > 1 || !strncmp(filename, "/", 1)) {
        //  printf("Path: %s\nFilename: %s\n", path, prevtoken);
        change_dir(path, &disk->cur_dir, disk);
        cur_dir = disk->cur_dir;
        memset(filename, 0, strlen(filename) + 1);
        strncpy(filename, prevtoken, strlen(prevtoken) + 1);
    }

    FileHead* head = open_file(filename, cur_dir, disk);
    if (!head && DEBUG) {
        printf("Error opening file\n");
        return -1;
    }
    if(head->pos == -1)
        head->pos = head->size;
    // printf("Opened file with head %p\n", head);
    if (pos && (pos >= head->size || pos < -2)) {
        printf("Error: position is invalid!\n");
        return -1;
    }

    int sum = 0;    // sum of written bytes
    FatEntry* block = get_fat_entry_ptr(head->idx, disk);
    // printf("Head points to %p in the FAT\n", block);
    FatEntry* prev_block;   // to store the previous FAT block when allocating a new one
    File* file;     // pointer to the disk block
    int next_idx;   // to update the FAT after allocating a new block
    int buf_pos = 0;    // current position in the input buffer

    int input_len = 0;
    for (int i = 0; buf[i]; i++)
        input_len++;

    if (!n_bytes) {
        int len = 0;
        for (int i = 0; buf[i]; i++)
            len++;
        n_bytes = len;
    }
    if (n_bytes > input_len)
        n_bytes = input_len;
    if (pos == -1)
        pos = head->size;
    if (pos == -2)
        pos = head->pos;
    int block_num = pos / (BLOCK_SIZE - 2*sizeof(int));
    int block_offset = pos % (BLOCK_SIZE - 2*sizeof(int));

    // printf("To write: %d bytes\n", n_bytes);

    while (1) {
        int j = 0;
        int idx;
        if (!sum) {
            for (int i = 0; i < block_num; i++) {
                idx = block->data;
                block = &disk->fat.array[idx];
            }
            idx = block->data;
            // printf("Writing in FAT %d\n", idx);
            file = get_file_ptr(disk->fat.array[idx].file, disk);
            if (!file) {
                printf("Error retreiving file index!\n");
                return -1;
            }
            // printf("First iteration: file points to %p in the disk (%d in FAT, next is %d)\n", file, head->start->idx, idx);
        } else {
            idx = next_idx;
            // printf("Writing in FAT %d\n", idx);
            file = get_file_ptr(disk->fat.array[idx].file, disk);
            if (!file) {
                printf("Error retreiving file index!\n");
                return -1;
            }
            // printf("File points to %p in the disk (%d in FAT, next is %d)\n", file, head->start->idx, idx);
        }
        FatEntry* entry_ptr = get_fat_entry_ptr(file->idx, disk);
        block = entry_ptr;
        // write until the block is full
        if (DEBUG)
            printf("Writing in block (file: %s\taddress: %p\tidx: %d\tdata: %p)...\n", head->name, file, file->idx, file->data);
        // printf("Free bytes in block: %d\n", file->free_in_block);
        char* block_end = file->data + sizeof(file->data);
        // printf("Block size: %ld\n", sizeof(file->data));
        
        while (sum < n_bytes && &file->data[j + block_offset] < block_end) {
            int override = 0;
            if (file->data[j + block_offset]) {
                override = 1;
            }
            file->data[j + block_offset] = buf[buf_pos];
            if (DEBUG) printf("%c", file->data[j + block_offset]);
            j++;
            buf_pos++;
            sum++;
            if (!override)
                head->size++;
            if (file->free_in_block) {
                file->free_in_block--;
            }
            if (!override && !file->free_in_block)
                break;
        }
        if (DEBUG) printf("\n");
        //printf("Written %d bytes\n", sum);
        // if we are done we exit
        if (sum == n_bytes)
            break;
        // otherwise go to the next block or we allocate a new one
        entry_ptr = get_fat_entry_ptr(file->idx, disk);
        if (entry_ptr->data == -1) {
            if (DEBUG)
                printf("Requesting new block...\n");
            prev_block = block;
            block = request_fat_blocks(disk, prev_block, 1);
            int index = get_fat_entry_idx(disk, block);
            if (index == -1) {
                printf("Error getting FAT entry index!\n");
                return -1;
            }
            prev_block->data = index;
            next_idx = index;
            file = (File*) find_block(disk);
            file->idx = get_block_idx(disk);
            block->file = file->idx;
            file->free_in_block = BLOCK_SIZE - 2*sizeof(int);
            block->file = file->idx;
        } else {
            block = &disk->fat.array[entry_ptr->data];
            next_idx = get_fat_entry_idx(disk, block);
            if (next_idx == -1) {
                printf("Error getting FAT entry index!\n");
                return -1;
            }
        }
        block_offset = 0;
    }
    printf("Successfully written %d bytes in %s\n", sum, filename);
    return sum;
}

int seek_in_file(char* filename, int pos, int cur_dir, Disk* disk) {
    if (pos < 0 && pos != -1) {
        printf("Position not valid!\n");
        return -1;
    }

    char filenamecpy[MAX_PATH_LENGTH];
    strncpy(filenamecpy, filename, strlen(filename) + 1);

    // Dir* cur_dir_ptr = get_dir_ptr(cur_dir, disk);
    if (!file_exists(filename, cur_dir, disk)) {
        printf("Unable to seek in file: %s doesn't exist in the current directory!\n", filename);
        return -1;
    }

    //printf("filenamecpy: %s\n", filenamecpy);
    char path[MAX_PATH_LENGTH];
    char prevtoken[30];
    int token_count = 0;
    char* token = strtok(filenamecpy, "/");
    if (token)
        token_count++;
    memset(path, 0, strlen(path) + 1);
    //printf("path: %s\n", path);
    //printf("token: %s\n", token);
    while(token) {
        strncpy(prevtoken, token, strlen(token) + 1);
        //printf("prevtoken: %s\n", prevtoken);
        token = strtok(NULL, "/");
        if (token) {
            token_count++;
            if (token_count > 1 && strncmp(filename, "..", 2))
                strncat(path, "/", 2);
            strncat(path, prevtoken, strlen(prevtoken) + 1);
            //printf("path: %s\n", path);
        }
        //printf("token: %s\n", token);
    }
    if (token_count > 1 || !strncmp(filename, "/", 1)) {
        //  printf("Path: %s\nFilename: %s\n", path, prevtoken);
        change_dir(path, &disk->cur_dir, disk);
        cur_dir = disk->cur_dir;
        memset(filename, 0, strlen(filename) + 1);
        strncpy(filename, prevtoken, strlen(prevtoken) + 1);
    }

    FileHead* file = open_file(filename, cur_dir, disk);
    if (!file)
        handle_error("Error opening file");
    if (pos > file->size) {
        printf("Position not valid!\n");
        return -1;
    }
    if (pos == -1)
        file->pos = file->size-1;
    file->pos = pos;
    if (file->pos == -1)
        printf("Current position in file '%s': end\n", filename);
    else
        printf("Current position in file '%s': %d\n", filename, file->pos);
    return file->pos;
}

void print_cur_dir(Disk* disk) {
    //Dir* cur_dir_ptr = get_dir_ptr(cur_dir, disk);
    printf("Current directory: %s\n", disk->cur_path);
}