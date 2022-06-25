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