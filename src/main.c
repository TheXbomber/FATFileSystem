#include "disk.h"
#include "fat.h"
#include "file.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main (int argc, char** argv) {
    char* buffer;
    Disk* disk = disk_init("my_disk.img", buffer);
    disk_print(disk, buffer);

    // int ret = create_file(disk, "test.txt");
    // FatEntry* block = request_blocks(disk, 5);
    // disk_print(disk, buffer);

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