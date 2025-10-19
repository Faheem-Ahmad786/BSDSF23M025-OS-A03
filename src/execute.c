#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>

void execute(char **arglist) {
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork failed");
        exit(1);
    }

    if (pid == 0) {
        int in_redirect = -1, out_redirect = -1;
        char *infile = NULL, *outfile = NULL;

        // 🔹 Check for redirection symbols
        for (int i = 0; arglist[i] != NULL; i++) {
            if (strcmp(arglist[i], "<") == 0) {
                infile = arglist[i + 1];
                in_redirect = i;
            } else if (strcmp(arglist[i], ">") == 0) {
                outfile = arglist[i + 1];
                out_redirect = i;
            }
        }

        // 🔹 Handle input redirection (<)
        if (infile != NULL) {
            int fd = open(infile, O_RDONLY);
            if (fd < 0) {
                perror("Input file open failed");
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        // 🔹 Handle output redirection (>)
        if (outfile != NULL) {
            int fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("Output file open failed");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        // 🔹 Remove redirection symbols from arglist
        if (in_redirect != -1) arglist[in_redirect] = NULL;
        if (out_redirect != -1) arglist[out_redirect] = NULL;

        // 🔹 Execute the command
        execvp(arglist[0], arglist);
        perror("Command execution failed");
        exit(1);
    } 
    else {
        wait(NULL);
    }
}

