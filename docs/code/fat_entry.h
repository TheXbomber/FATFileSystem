typedef struct {
    int file;               // index of the disk block
    int data;               // index of next block
    int busy;               // 0 if the block is free
    int idx;                // index of this block
} FatEntry;