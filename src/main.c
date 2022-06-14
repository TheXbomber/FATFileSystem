#include "disk.h"
#include "fat.h"
#include "file.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main (int argc, char** argv) {
    Disk* disk = disk_init("my_disk.img");
    disk_print(disk);

    // int ret = create_file(disk, "test.txt");
    FatEntry* block = request_blocks(disk, 5);
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