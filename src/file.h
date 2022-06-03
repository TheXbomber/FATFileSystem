#pragma once
#include "fat.h"
#include "disk.h"
#include <math.h>

typedef struct {
    char* name;
    int size;
    FatEntry* start;    // pointer to first block of file
    int num_blocks;     // =(int)ceil(size/BLOCK_SIZE);
    char is_dir;        // 1 if the file is a directory
    char* data;         // data contained in the file
} File;

typedef struct {
    File file;          // a directory is a file with no data
    int num_files;
    File* files;        // list of file in directory
} Dir;