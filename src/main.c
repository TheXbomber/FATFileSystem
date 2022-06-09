#include "disk.h"
#include "fat.h"
#include "file.h"
#include <stdio.h>

int main (int argc, char** argv) {
    char* buffer;
    Disk* disk = disk_init("my_disk.img", buffer);
    disk_print(disk, buffer);

    // int ret = create_file(disk, "test.txt");
    // disk_print(disk, buffer);
    return 0;
}