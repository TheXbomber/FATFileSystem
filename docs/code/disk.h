typedef struct {
    int size;       // size of the disk array
    int cur_dir;    // pointer to current directory
    int root_dir;   // pointer to the root directory
    Fat fat;        // pointer to start of the FAT
    char data[DISK_SIZE - 3*sizeof(int) - sizeof(Fat)];
} Disk;