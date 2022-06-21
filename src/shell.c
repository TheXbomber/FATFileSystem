#include "headers/error.h"
#include "headers/disk.h"
#include "headers/fat.h"
#include "headers/file.h"
#include <stdio.h>
#include <string.h>
//#include <stdlib.h>

int main(int argc, char** argv) {
    Dir* cur_dir = NULL;
    char* buffer = map_file("my_disk.img");
    Disk* disk = disk_init(buffer, 1);
    if (DEBUG)
        printf("Creating root directory...\n");
    Dir* dir = create_dir("/", NULL, disk);
    if (!dir)
        handle_error("error creating root directory");
    cur_dir = dir;

    // char* line = malloc(100);
    // char** args = malloc(100*sizeof(char*));
    // for (int i = 0; i < 100; i++)
    //     args[i] = malloc(101);
    // char* cmd = malloc(100);
    // char* token = malloc(100);

    printf("*** FAT File System Shell ***\n");
    printf("Type \"help\" for a list of available commands\n");
    while(1) {
        printf("%s> ", cur_dir->name);
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

        // // printf("cmd: %s\n", cmd);
        // // printf("args:");
        // // for (int i = 0; args && args[i]; i++) {
        // //     printf(" %s", args[i]);
        // // }
        // // printf("\n");

        // fgets(line, 100, stdin);
        // printf("Line: %s\n", line);
        // token = strtok(line, " \n");
        // for (int i = 0; i < 5 && token != NULL; i++) {
        //     if (i == 0) {
        //         cmd = token;
        //         printf("Cmd: %s\n", cmd);
        //     } else {
        //         args[i] = token;
        //         printf("Arg%d: %s\n", i, args[i]);
        //     }
        //     token = strtok(NULL, " \n");
        // }

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
            create_file((char*) args[0], cur_dir, disk);
        } else if (!strcmp(cmd, "rm")) {
            delete_file((char*) args[0], cur_dir, disk);
        } else if (!strcmp(cmd, "rd")) {
            read_file((char*) args[0], atoi(args[1]), atoi(args[2]), cur_dir, disk);
        } else if (!strcmp(cmd, "wr")) {
            printf("Input for write ('\\n' to confirm):\n");
            char buf[2048] = {};
            char cc = (char) fgetc(stdin);
            for (int i = 0; cc != '\n'; i++) {
                buf[i] = cc;
                cc = (char) fgetc(stdin);
            }
            write_file(args[0], buf, atoi(args[1]), atoi(args[2]), cur_dir, disk);
        } else if (!strcmp(cmd, "seek")) {
            seek_in_file((char*) args[0], atoi(args[1]), cur_dir, disk);
        } else if (!strcmp(cmd, "mkdir")) {
            create_dir((char*) args[0], cur_dir, disk);
        } else if (!strcmp(cmd, "rmdir")) {
            delete_dir((char*) args[0], cur_dir, disk);
        } else if (!strcmp(cmd, "cd")) {
            change_dir((char*) args[0], &cur_dir);
        } else if (!strcmp(cmd, "ls")) {
            list_dir(cur_dir);
        } else if (!strcmp(cmd, "pwd")) {
            print_cur_dir(cur_dir);
        } else if (!strcmp(cmd, "disk_print")) {
            disk_print(disk);
        } else {
            printf("Command \"%s\" not recognized!\n", cmd);
        }
    }

    // free(line);
    // free(token);
    // free(cmd);
    // for (int i = 0; i < 100; i++) {
    //     free(args[i]);
    // }
    // free(args);

    return 0;
}