typedef struct {
    int free_blocks;                 // number of free blocks
    FatEntry array[FAT_BLOCKS_MAX];  // FAT
} Fat;