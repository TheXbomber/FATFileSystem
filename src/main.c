#include "headers/error.h"
#include "headers/disk.h"
#include "headers/fat.h"
#include "headers/file.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main (int argc, char** argv) {
    char* buffer = map_file("my_disk.img");
    Disk* disk = disk_init(buffer, 1);
    if (DEBUG)
        printf("Creating root directory...\n");
    Dir* dir = create_dir("/", 0, disk);
    if (!dir)
        handle_error("error creating root directory");
    disk->root_dir = dir->idx;
    disk->cur_dir = disk->root_dir;
    //disk_print(disk);

    int ret;
    char* input;

    ret = create_file("file1.txt", disk->cur_dir, disk);
    ret = create_file("file2.txt", disk->cur_dir, disk);
    create_dir("dir1", disk->cur_dir, disk);
    list_dir(disk->cur_dir, disk);
    ret = delete_dir("dir1", disk->cur_dir, disk);
    //ret = delete_file("file1.txt", disk->cur_dir, disk);
    list_dir(disk->cur_dir, disk);

    disk_print(disk);

    input = "ciao ciao";
    ret = write_file("file2.txt", input, 0, 0, disk->cur_dir, disk);
    printf("Written %d bytes\n", ret);
    list_dir(disk->cur_dir, disk);
    ret = read_file("file2.txt", 0, 0, disk->cur_dir, disk);
    printf("Read %d bytes\n", ret);

    // printf("Disk data:\n");
    // for (int i = 0; i < BLOCK_SIZE; i++) {
    //     printf("%c", disk->data[i]);
    // }
    // printf("\n");

    return 0;
}