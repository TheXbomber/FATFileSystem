struct Dir {
    int idx;            // index of the disk block
    char name[30];
    int is_dir;         // 1
    int parent_dir;     // index of parent directory
    int num_files;      // number of files
    int num_dirs;       // number of subdirectories
    int start;          // position in the FAT   
    int files[BLOCK_SIZE - 30*sizeof(char) - 5*sizeof(int)];
    // list of files and subdirectories
};