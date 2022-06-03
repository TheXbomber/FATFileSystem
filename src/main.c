#include "disk.h"
#include "fat.h"
#include "file.h"
#include <stdio.h>

int main (int argc, char** argv) {
    Disk* disk = disk_init("my_disk.img");
    disk_print(disk);
    return 0;
}