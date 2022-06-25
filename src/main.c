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

    // to test stuff...

    return 0;
}