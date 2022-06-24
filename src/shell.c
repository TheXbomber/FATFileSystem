#include "headers/error.h"
#include "headers/disk.h"
#include "headers/fat.h"
#include "headers/file.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

int main(int argc, char** argv) {
    Disk* disk;
    char* buffer;
    if (!access("my_disk.img", F_OK)) {
        buffer = map_file("my_disk.img");
        printf("Initialized disk found\n");
        disk = disk_init(buffer, 0);
    } else {
        buffer = map_file("my_disk.img");
        disk = disk_init(buffer, 1);
        if (DEBUG)
            printf("Creating root directory...\n");
        Dir* dir = create_dir("/", 0, disk);
        if (!dir)
            handle_error("error creating root directory");
        disk->root_dir = dir->idx;
        disk->cur_dir = disk->root_dir;
    }

    printf("*** FAT File System Shell ***\n");
    printf("Type \"help\" for a list of available commands\n");
    while(1) {
        Dir* cur_dir_ptr = get_dir_ptr(disk->cur_dir, disk);
        printf("%s> ", cur_dir_ptr->name);
        // printf("> ");
        char cmd[100] = {};
        char args[100][100] = {};
        char c = (char) fgetc(stdin);
        int i = 0;
        int j = 0;
        int k = 0;

        while (c != '\n' && c != ' ') {
            cmd[i] = c;
            c = (char) fgetc(stdin);
            i++;
        }
        if (c != '\n') {
            c = (char) fgetc(stdin);
            while (c != '\n') {
                if (c != ' ') {
                    args[j][k] = c;
                    k++;
                } else {
                    if (DEBUG)
                        printf("arg%d: %s\n", j, args[j]);
                    j++;
                    k = 0;
                }
                c = (char) fgetc(stdin);
            }
            if (DEBUG)
                printf("arg%d: %s\n", j, args[j]);
        }

        if (!strcmp(cmd, "help")) {
            // help function
            printf("Here's a list of all available commands:\n");
            printf(" - help : get a list of all available commands\n");
            printf(" - quit : close the program\n");
            printf(" - touch <filename> : create a file\n");
            printf(" - rm <filename> : delete a file\n");
            printf(" - rd <filename> <pos> <n> : read n bytes (0 for all) from a file starting from position pos (-1 for current)\n");
            printf(" - wr <filename> <pos> <n> : write n bytes (0 for all) in a file starting from position pos (-1 for current)\n");
            printf(" - seek <filename> <pos> : change the current position of a file to pos (-1 for file end)\n");
            printf(" - mkdir <dirname> : create a directory\n");
            printf(" - rmdir <dirname> : delete a directory and all of its content\n");
            printf(" - cd <dirname> : open a directory\n");
            printf(" - ls : list the content of the current directory\n");
            printf(" - pwd : print the current directory\n");
        } else if (!strcmp(cmd, "quit")) {
            // quit function
            printf("Exiting...\n");
            break;
        } else if (!strcmp(cmd, "touch")) {
            create_file((char*) args[0], disk->cur_dir, disk);
        } else if (!strcmp(cmd, "rm")) {
            delete_file((char*) args[0], disk->cur_dir, disk);
        } else if (!strcmp(cmd, "rd")) {
            read_file((char*) args[0], atoi(args[1]), atoi(args[2]), disk->cur_dir, disk);
        } else if (!strcmp(cmd, "wr")) {
            printf("Input for write ('\\n' to confirm):\n");
            char buf[2048] = {};
            char cc = (char) fgetc(stdin);
            for (int i = 0; cc != '\n'; i++) {
                buf[i] = cc;
                cc = (char) fgetc(stdin);
            }
            write_file(args[0], buf, atoi(args[1]), atoi(args[2]), disk->cur_dir, disk);
        } else if (!strcmp(cmd, "seek")) {
            seek_in_file((char*) args[0], atoi(args[1]), disk->cur_dir, disk);
        } else if (!strcmp(cmd, "mkdir")) {
            create_dir((char*) args[0], disk->cur_dir, disk);
        } else if (!strcmp(cmd, "rmdir")) {
            delete_dir((char*) args[0], disk->cur_dir, disk);
        } else if (!strcmp(cmd, "cd")) {
            change_dir((char*) args[0], &disk->cur_dir, disk);
        } else if (!strcmp(cmd, "ls")) {
            list_dir(disk->cur_dir, disk);
        } else if (!strcmp(cmd, "pwd")) {
            print_cur_dir(disk->cur_dir, disk);
        } else if (!strcmp(cmd, "disk_print")) {
            disk_print(disk);
        } else {
            printf("Command \"%s\" not recognized!\n", cmd);
        }
    }

    munmap(buffer, DISK_SIZE);

    return 0;
}