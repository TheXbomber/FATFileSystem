#include "error.h"
#include "disk.h"
#include "fat.h"
#include "file.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

Dir* cur_dir = NULL;    // NEEDS FIXING

void print_cur_dir() {
    printf("Current directory: %s\n", cur_dir->name);
}

int main (int argc, char** argv) {
    Disk* disk = disk_init("my_disk.img");
    if (DEBUG)
        printf("Creating root directory...\n");
    int ret = create_dir(disk, cur_dir, "/");
    if (ret)
        handle_error("error creating root directory");
    disk_print(disk);

    ret = create_file(disk, cur_dir, "test1.txt");
    disk_print(disk);

    ret = create_file(disk, cur_dir, "test2.txt");
    disk_print(disk);

    ret = create_dir(disk, cur_dir, "dir");
    disk_print(disk);

    printf("READ TEST\n");
    int fd = open("my_disk.img", O_RDWR, 0666);
    if (!fd)
        printf("Can't open\n");
    char buf[DISK_SIZE];
    int res = read(fd, &buf, disk->size);
    if (!res)
        printf("Can't read\n");
    for (int i = 0; buf[i]; i++) {
        printf("%d: %c\t", i, buf[i]);
    }
    printf("\n");

    return 0;
}