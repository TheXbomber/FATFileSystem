typedef struct {
    int idx;
    char name[30];
    int is_dir;         // 0
    int size;
    int pos;            // current position in the file
    int parent_dir;    // directory that stores the file
    int start;    // pointer to first block of file
} FileHead;