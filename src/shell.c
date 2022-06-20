#include "error.h"
#include "disk.h"
#include "fat.h"
#include "file.h"
#include <stdio.h>
#include <string.h>

Dir* cur_dir = NULL;

void print_cur_dir() {
    printf("Current directory: %s\n", cur_dir->name);
}

int main(int argc, char** argv) {
    char* buffer = map_file("my_disk.img");
    Disk* disk = disk_init(buffer, 1);
    if (DEBUG)
        printf("Creating root directory...\n");
    Dir* dir = create_dir(disk, NULL, "/");
    if (!dir)
        handle_error("error creating root directory");
    cur_dir = dir;

    printf("*** FAT File System Shell ***\n");
    printf("Type \"help\" for a list of available commands\n");
    while(1) {
        char cmd[100] = {};
        char args[100][100] = {};
        printf("> ");
        char c = fgetc(stdin);
        int i = 0;
        int j = 0;
        int k = 0;

        while (c != '\n' && c != ' ') {
            cmd[i] = c;
            c = fgetc(stdin);
            i++;
        }
        if (c != '\n') {
            c = fgetc(stdin);
            while (c != '\n') {
                if (c != ' ') {
                    args[j][k] = c;
                    k++;
                } else {
                    printf("arg%d: %s\n", j, args[j]);
                    j++;
                    k = 0;
                }
                c = fgetc(stdin);
            }
            printf("arg%d: %s\n", j, args[j]);
        }

        // printf("cmd: %s\n", cmd);
        // printf("args:");
        // for (int i = 0; args && args[i]; i++) {
        //     printf(" %s", args[i]);
        // }
        // printf("\n");

        if (!strcmp(cmd, "help")) {
            // help function
            printf("Here's a list of all available commands:\n");
            printf(" - help : get a list of all available commands\n");
            printf(" - quit : close the program\n");
            printf(" - touch <filename> : create a file\n");
            printf(" - rm <filename> : delete a file\n");
            printf(" - rd <filename> <pos> <n> : read n bytes from a file starting from position pos\n");
            printf(" - wr <filename> <pos> <n> : write n bytes in a file starting from position pos\n");
            printf(" - seek <filename> <pos> : change the current position of a file to pos\n");
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
            create_file(disk, cur_dir, args[0]);
        } else if (!strcmp(cmd, "rm")) {
            delete_file(args[0], cur_dir, disk);
        } else if (!strcmp(cmd, "rd")) {
            read_file(args[0], (int) args[1], (int) args[2], cur_dir, disk);
        } else if (!strcmp(cmd, "wr")) {
            printf("Input for write ('\n' to confirm):\n");
            char buf[2048] = {};
            char cc = fgetc(stdin);
            for (int i = 0; cc != '\n'; i++) {
                buf[i] = cc;
                cc = fgetc(stdin);
            }
            write_file(args[0], buf, (int) args[1], (int) args[2], cur_dir, disk);
        } else if (!strcmp(cmd, "seek")) {
            seek_in_file(args[0], (int) args[1], cur_dir, disk);
        } else if (!strcmp(cmd, "mkdir")) {
            create_dir(disk, cur_dir, args[0]);
        } else if (!strcmp(cmd, "rmdir")) {
            delete_dir(args[0], disk, cur_dir);
        } else if (!strcmp(cmd, "cd")) {
            change_dir(args[0], &cur_dir);
        } else if (!strcmp(cmd, "ls")) {
            list_dir(cur_dir);
        } else if (!strcmp(cmd, "pwd")) {
            print_cur_dir();
        } else {
            printf("Command \"%s\" not recognized!\n", cmd);
        }
    }

    return 0;
}