#pragma once
#include "fat.h"
#include "disk.h"

typedef struct Dir Dir;

typedef struct {
    int idx;
    char name[30];
    int is_dir;         // 0
    int size;
    int pos;            // current position in the file
    int parent_dir;    // directory that stores the file
    int start;    // pointer to first block of file
} FileHead;

struct Dir {
    int idx;
    char name[30];
    int is_dir;         // 1
    int parent_dir;
    int num_files;      // number of files
    int num_dirs;       // number of subdirectories
    int start;    // position in the FAT   
    int files[BLOCK_SIZE - 30*sizeof(char) - 5*sizeof(int)];   // list of files in directory
};

typedef struct File {
    int idx;
    int free_in_block;
    char data[BLOCK_SIZE - 2*sizeof(int)];
} File;

// creates a file named filename and returns 0
int create_file(char* filename, int parent_dir, Disk* disk);

// deletes the file filename and returns 0
int delete_file(char* filename, int cur_dir, Disk* disk);

// reads the file filename from position pos (-1 for current position) and returns number of bytes read
int read_file(char* filename, int pos, int n_bytes, int cur_dir, Disk* disk);

// writes n_bytes bytes from buf in the file filename from position pos (-1 for current position) and returns the number of bytes written
int write_file(char* filename, char* buf, int pos, int n_bytes, int cur_dir, Disk* disk);

// change to position pos in the file filename and return the postion
int seek_in_file(char* filename, int pos, int cur_dir, Disk*);

// creates a directory called dir_name and returns it
Dir* create_dir(char* dirname, int parent_dir, Disk* disk);

// deletes the directory dir and returns 0
int delete_dir(char* dirname, int cur_dir, Disk* disk);
// auxiliary function for delete_dir
int delete_dir_aux(Disk* disk, Dir* cur_dir, Dir* dir, int idx);

// opens the dir directory and returns 0
int change_dir(char* dirname, int* cur_dir, Disk* disk);

// lists the directory in the current position and returns the number of directories listed
int list_dir(int dir, Disk* disk);

// returns 1 if a file named filename already exists in the current directory
int file_exists(char* filename, int cur_dir, Disk* disk);

// returns 1 if a dir named dirname already exists in the current directory
int dir_exists(char* dirname, int cur_dir, Disk* disk);

// opens a file and returns its header
FileHead* open_file(char* filename, int cur_dir, Disk* disk);

// prints the current directory
void print_cur_dir(int cur_dir, Disk* disk);

// returns the pointer to the FAT block relative to the disk block with index idx
FatEntry* get_fat_entry_ptr(int idx, Disk* disk);

// returns the pointer to the file header relative to the disk block with index idx
FileHead* get_file_head_ptr(int idx, Disk* disk);

// returns the pointer to the disk block with index idx
File* get_file_ptr(int idx, Disk* disk);

// returns the pointer to the directory relative to the disk block with index idx
Dir* get_dir_ptr(int idx, Disk* disk);