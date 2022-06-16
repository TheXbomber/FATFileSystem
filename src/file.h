#pragma once
#include "fat.h"
#include "disk.h"

typedef struct Dir Dir;

typedef struct {
    int pos;            // current position in a file
} FileHandle;

typedef struct {
    FileHandle* handle; // must be the first
    int is_dir;         // 0
    char* name;
    int size;
    Dir* parent_dir;   // directory that stores the file
    FatEntry* start;    // pointer to first block of file
    char* data;
} FileHead;

struct Dir {
    int is_dir;         // 1
    char* name;
    Dir* parent_dir;
    int num_files;      // number of files
    FatEntry* start;    // position in the FAT    
    FileHead* files;    // list of files in directory
};

typedef struct {
    FatEntry* block;
    char* data;
} File;

// create a file named filename
int create_file(Disk* disk, Dir* dir, char* filename);

// delete the file filename
int delete_file(char filename);

// read the file filename
FileHandle* read_file(char* filename);

// write n_bytes bytes in the file filename
int write_file(char* filename, int n_bytes);

// change to position pos in the file filename
char* seek_in_file(char* filename, int pos);

// create a directory called dir_name
int create_dir(Disk* disk, Dir* dir, char* dirname);

// delete the directory dir
int delete_dir(Disk* disk, char* dirname);

// open the dir directory
int change_dir(char* dirname);

// list the directory in the current position
int list_dir();

// checks if a file named filename already exists
int file_exists(char* filename);

// prints the current directory
void print_cur_dir();