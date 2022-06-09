#pragma once
#include "fat.h"
#include "disk.h"

typedef struct {
    char* name;
    int size;
    FatEntry* start;    // pointer to first block of file
    char is_dir;        // 1 if the file is a directory
} File;

typedef struct {
    File file;          // a directory is a file with no data
    int num_files;
    File* files;        // list of file in directory
} Dir;

typedef struct {
    int pos;            // current position in a file
} FileHandle;

// create a file named filename
int create_file(Disk* disk, char* filename);

// delete the file filename
int delete_file(char filename);

// read the file filename
FileHandle* read_file(char* filename);

// write n_bytes bytes in the file filename
int write_file(char* filename, int n_bytes);

// change to position pos in the file filename
char* seek_in_file(char* filename, int pos);

// create a directory called dir_name
int create_dir(char* dirname);

// delete the directory dir
int delete_dir(char* dirname);

// open the dir directory
int change_dir(char* dirname);

// list the directory in the current position
int list_dir();

// checks if a file named filename already exists
int file_exists(char* filename);