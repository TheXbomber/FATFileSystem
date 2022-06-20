#include "error.h"
#include "disk.h"
#include "fat.h"
#include "file.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

Dir* cur_dir = NULL;

void print_cur_dir() {
    printf("Current directory: %s\n", cur_dir->name);
}

int main (int argc, char** argv) {
    char* buffer = map_file("my_disk.img");
    Disk* disk = disk_init(buffer, 1);
    if (DEBUG)
        printf("Creating root directory...\n");
    Dir* dir = create_dir(disk, NULL, "/");
    if (!dir)
        handle_error("error creating root directory");
    cur_dir = dir;
    //disk_print(disk);

    int ret;
    char* input;

    // ret = create_file(disk, cur_dir, "test1.txt");
    //disk_print(disk);

    //ret = create_file(disk, cur_dir, "test2.txt");
    //ret = create_file(disk, cur_dir, "test2.txt");  // can't create
    //disk_print(disk);

    //list_dir(cur_dir);

    // ret = read_file("test1.txt", cur_dir, disk);
    // ret = read_file("nonexistant.txt", cur_dir, disk);  // can't read
    // if (DEBUG && ret != -1) {
    //     printf("Read %d bytes\n", ret);
    // }

    // dir = create_dir(disk, cur_dir, "dir1");
    // dir = create_dir(disk, cur_dir, "dir1");        // can't create
    // dir = create_dir(disk, cur_dir, "dir2");
    // list_dir(cur_dir);

    // print_cur_dir();
    // change_dir("..", &cur_dir);     // can't change
    // change_dir("dir2", &cur_dir);
    // print_cur_dir();
    // list_dir(cur_dir);
    // change_dir("..", &cur_dir);
    // print_cur_dir();

    // input = "TEST_SCRITTURA_SU_FILE";
    // ret = write_file("test2.txt", input, 22, cur_dir, disk);
    // if (DEBUG && ret != -1) {
    //     printf("Written %d bytes\n", ret);
    // }
    // // disk_print(disk);
    // ret = read_file("test2.txt", cur_dir, disk);
    // if (DEBUG && ret != -1) {
    //     printf("Read %d bytes\n", ret);
    // }

    // input = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed dignissim consectetur odio sed malesuada. Nunc bibendum, arcu non semper varius, ipsum ante mattis massa, id facilisis ante lectus eu lorem. Nullam lectus lectus, pretium eu varius non, maximus sit amet lectus. Integer consectetur lectus nec libero facilisis tempor. Curabitur et nulla quis massa sollicitudin aliquam. Mauris semper velit leo, at dapibus odio ultrices eu. Sed euismod finibus enim, at porttitor nisl sodales non. Vestibulum sed lectus turpis.";
    // ret = write_file("test1.txt", input, 520, cur_dir, disk);
    // if (DEBUG && ret != -1) {
    //     printf("Written %d bytes\n", ret);
    // }
    // disk_print(disk);
    // ret = read_file("test1.txt", cur_dir, disk);
    // if (DEBUG && ret != -1) {
    //     printf("Read %d bytes\n", ret);
    // }

    ret = create_file(disk, cur_dir, "test3.txt");
    input = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Cras molestie suscipit ante sed viverra. Integer arcu mauris, mollis quis gravida egestas, malesuada a ex. Duis ultrices ipsum nec dignissim posuere. Suspendisse sed lorem sed augue venenatis dignissim eget consequat mauris. Proin non tristique neque, eget cursus urna. Integer fringilla blandit dolor, vel convallis purus convallis et. Interdum et malesuada fames ac ante ipsum primis in faucibus. Phasellus hendrerit varius libero quis pellentesque. Vestibulum faucibus felis quam, sit amet consectetur felis vulputate sit amet. Morbi placerat ex in lobortis sagittis. Aliquam eget ex vel nisl tempor maximus. Donec quis elit eu nunc iaculis accumsan ac ac mauris. Mauris accumsan aliquam erat ut pulvinar. Proin faucibus mauris sit amet ex mattis pellentesque. Mauris condimentum arcu turpis, sed accumsan nibh facilisis a. Maecenas erat orci, venenatis nec rutrum non, aliquam nec odio. Quisque nulla velit, congue commodo sem vel, fermentum consectetur eros. Vivamus a orci sit.";
    ret = write_file("test3.txt", input, 0, 0, cur_dir, disk);
    if (DEBUG && ret != -1) {
        printf("Written %d bytes\n", ret);
    }
    input = "NUOVO_TESTO";  // 972, 1030
    ret = write_file("test3.txt", input, 970, 0, cur_dir, disk);
    //disk_print(disk);
    ret = read_file("test3.txt", 0, 0, cur_dir, disk);
    if (DEBUG && ret != -1) {
        printf("Read %d bytes\n", ret);
    }

    // seek_in_file("test3.txt", 540, cur_dir, disk);

    // ret = delete_file("test1.txt", cur_dir, disk);

    // disk_print(disk);

    // delete_dir("dir1", disk, cur_dir);

    // disk_print(disk);

    // printf("READ TEST\n");
    // int fd = open("my_disk.img", O_RDONLY, 0444);
    // if (!fd)
    //     printf("Can't open\n");
    // char buf[DISK_SIZE];
    // int res = read(fd, &buf, DISK_SIZE);
    // if (!res)
    //     printf("Can't read\n");
    // for (int i = 0; i < DISK_SIZE; i++) {
    //     printf("%c", buf[i]);
    // }
    // printf("\n");

    return 0;
}