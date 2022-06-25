typedef struct File {
    int idx;
    int free_in_block;
    char data[BLOCK_SIZE - 2*sizeof(int)];
} File;