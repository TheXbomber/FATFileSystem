typedef struct {
    int file;               // index of the disk block
    int data;               // index of next block
} FatEntry;