#pragma once
#include "fat.h"
#include "disk.h"

typedef struct Dir Dir;

typedef struct {
    int pos;            // current position in a file
} FileHandle;

typedef struct {
    char* name;
    int is_dir;         // 0
    int size;
    // int free_in_block;  // free bytes in the current block
    int pos;            // current position in the file
    Dir* parent_dir;    // directory that stores the file
    FatEntry* start;    // pointer to first block of file
    //char* data;
} FileHead;

struct Dir {
    char* name;
    int is_dir;         // 1
    Dir* parent_dir;
    int num_files;      // number of files
    int num_dirs;       // number of subdirectories
    FatEntry* start;    // position in the FAT
    Dir** dirs;         // list of subdirectories    
    FileHead** files;   // list of files in directory
};

typedef struct File{
    FatEntry* block;
    int free_in_block;
    char* data;
} File;

// creates a file named filename and returns 0
int create_file(Disk* disk, Dir* parent_dir, char* filename);

// deletes the file filename and returns 0
int delete_file(char* filename, Dir* cur_dir, Disk* disk);

// reads the file filename from position pos (-1 for current position) and returns number of bytes read
int read_file(char* filename, int pos, int n_bytes, Dir* cur_dir, Disk* disk);

// writes n_bytes bytes from buf in the file filename from position pos (-1 for current position) and returns the number of bytes written
int write_file(char* filename, char* buf, int pos, int n_bytes, Dir* cur_dir, Disk* disk);

// change to position pos in the file filename and return the postion
int seek_in_file(char* filename, int pos, Dir* cur_dir, Disk*);

// creates a directory called dir_name and returns it
Dir* create_dir(Disk* disk, Dir* parent_dir, char* dirname);

// deletes the directory dir and returns 0
int delete_dir(char* dirname, Disk* disk, Dir* cur_dir);
// auxiliary function for delete_dir
int delete_dir_aux(Disk* disk, Dir* dir, int idx);

// opens the dir directory and returns 0
int change_dir(char* dirname, Dir** cur_dir);

// lists the directory in the current position and returns the number of directories listed
int list_dir(Dir* dir);

// returns 1 if a file named filename already exists in the current directory
int file_exists(char* filename, Dir* cur_dir);

// returns 1 if a dir named dirname already exists in the current directory
int dir_exists(char* dirname, Dir* cur_dir);

// opens a file and returns its header
FileHead* open_file(char* filename, Dir* cur_dir, Disk* disk);

// prints the current directory
void print_cur_dir();