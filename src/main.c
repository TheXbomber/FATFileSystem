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
    Dir* dir = create_dir(disk, NULL, "/");
    if (!dir)
        handle_error("error creating root directory");
    cur_dir = dir;
    //disk_print(disk);

    int ret = create_file(disk, cur_dir, "test1.txt");
    //disk_print(disk);

    ret = create_file(disk, cur_dir, "test2.txt");
    //disk_print(disk);

    list_dir(cur_dir);

    ret = read_file("test1.txt", cur_dir, disk);
    if (DEBUG)
        printf("Read %d bytes\n", ret);

    dir = create_dir(disk, cur_dir, "dir");
    //disk_print(disk);

    printf("READ TEST\n");
    int fd = open("my_disk.img", O_RDONLY, 0444);
    if (!fd)
        printf("Can't open\n");
    char buf[DISK_SIZE];
    int res = read(fd, &buf, DISK_SIZE);
    if (!res)
        printf("Can't read\n");
    for (int i = 0; i < DISK_SIZE; i++) {
        printf("%c", buf[i]);
    }
    printf("\n");

    return 0;
}