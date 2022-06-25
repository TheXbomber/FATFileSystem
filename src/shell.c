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
        if (DEBUG)
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

        if (!strncmp(cmd, "help", 4)) {
            // help function
            printf("Here's a list of all available commands:\n");
            printf(" - help : get a list of all available commands\n");
            printf(" - quit : close the program\n");
            printf(" - touch <filename> : create a file\n");
            printf(" - rmfile <filename> : delete a file\n");
            printf(" - rd <filename> <pos> <n> : read n bytes (0 for all) from a file starting from position pos (-1 for current)\n");
            printf(" - wr <filename> <pos> <n> : write n bytes (0 for all) in a file starting from position pos (-1 for current)\n");
            printf(" - seek <filename> <pos> : change the current position of a file to pos (-1 for file end)\n");
            printf(" - mkdir <dirname> : create a directory\n");
            printf(" - rmdir <dirname> : delete a directory and all of its content\n");
            printf(" - cd <dirname> : open a directory\n");
            printf(" - ls : list the content of the current directory\n");
            printf(" - pwd : print the current directory\n");
        } else if (!strncmp(cmd, "q", 1) || !strncmp(cmd, "quit", 4)) {
            // quit function
            printf("Exiting...\n");
            break;
        } else if (!strncmp(cmd, "touch", 5)) {
            if (!strcmp(args[0], ""))
                printf("Usage: touch <filename>\n");
            else
                create_file((char*) args[0], disk->cur_dir, disk);
        } else if (!strncmp(cmd, "rmfile", 6)) {
            if (!strcmp(args[0], ""))
                printf("Usage: rmfile <filename>\n");
            else
                delete_file((char*) args[0], disk->cur_dir, 0, disk);
        } else if (!strncmp(cmd, "rd", 2)) {
            if (!strncmp(args[0], "", MAX_ARG_LENGTH) || !strncmp(args[1], "", MAX_ARG_LENGTH) || !strncmp(args[2], "", MAX_ARG_LENGTH))
                printf("Usage: rd <filename> <position> <number of bytes>\n");
            else
                read_file((char*) args[0], atoi(args[1]), atoi(args[2]), disk->cur_dir, disk);
        } else if (!strncmp(cmd, "wr", 2)) {
            if (!strncmp(args[0], "", MAX_ARG_LENGTH) || !strncmp(args[1], "", MAX_ARG_LENGTH) || !strncmp(args[2], "", MAX_ARG_LENGTH))
                printf("Usage: wr <filename> <position> <number of bytes>\n");
            else {
                printf("Input to write (ENTER to confirm):\n");
                char buf[2048] = {};
                char cc = (char) fgetc(stdin);
                for (int i = 0; cc != '\n'; i++) {
                    buf[i] = cc;
                    cc = (char) fgetc(stdin);
                }
                write_file(args[0], buf, atoi(args[1]), atoi(args[2]), disk->cur_dir, disk);
            }
        } else if (!strncmp(cmd, "seek", 4)) {
            if (!strncmp(args[0], "", MAX_ARG_LENGTH) || !strncmp(args[1], "", MAX_ARG_LENGTH))
                printf("Usage: seek <filename> <position>\n");
            else
                seek_in_file((char*) args[0], atoi(args[1]), disk->cur_dir, disk);
        } else if (!strncmp(cmd, "mkdir", 5)) {
            if (!strncmp(args[0], "", MAX_ARG_LENGTH))
                printf("Usage: mkdir <dirname>\n");
            else
                create_dir((char*) args[0], disk->cur_dir, disk);
        } else if (!strncmp(cmd, "rmdir", 5)) {
            if (!strncmp(args[0], "", MAX_ARG_LENGTH))
                printf("Usage: rmdir <dirname>\n");
            else
                delete_dir((char*) args[0], disk->cur_dir, disk);
        } else if (!strncmp(cmd, "cd", 2)) {
            change_dir((char*) args[0], &disk->cur_dir, disk);
            // if (!strncmp(args[0], "", 1))
            //     path = "/";
            // else if (!strncmp(args[0], "..", 2))
            //     memset(path + strlen(path) - strlen(args[0]), 0, strlen(args[0]));
            // else {
            //     printf("ARG: %s\n", args[0]);
            //     if (strncmp(path, "/", 1)) {
            //         path = strncat(path, "/", 2);
            //         path = strncat(path, args[0], strlen(args[0])+1);
            //     }
            //     else
            //         path = strncat(path, args[0], strlen(args[0])+1);
            // }
        } else if (!strncmp(cmd, "ls", 2)) {
            list_dir(disk->cur_dir, disk);
        } else if (!strncmp(cmd, "pwd", 3)) {
            print_cur_dir(disk->cur_dir, disk);
        } else if (!strncmp(cmd, "disk_print", 10)) {
            disk_print(disk);
        } else {
            printf("Command \"%s\" not recognized!\n", cmd);
        }
    }

    munmap(buffer, DISK_SIZE);

    return 0;
}