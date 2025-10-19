#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>

// Helper: execute single or background process
void execute_single(char **arglist, int background) {
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork failed");
        exit(1);
    }

    if (pid == 0) {
        // Child process
        execvp(arglist[0], arglist);
        perror("Command execution failed");
        exit(1);
    } else {
        if (background) {
            printf("[%d] running in background\n", pid);
            return; // Don't wait for it
        } else {
            waitpid(pid, NULL, 0);
        }
    }
}

// Helper: split commands by '|'
int split_pipes(char **arglist, char ***commands) {
    int cmd_count = 0;
    commands[cmd_count] = arglist;

    for (int i = 0; arglist[i] != NULL; i++) {
        if (strcmp(arglist[i], "|") == 0) {
            arglist[i] = NULL; // terminate this command
            commands[++cmd_count] = &arglist[i + 1];
        }
    }
    return cmd_count + 1;
}

// Signal handler to notify when background process ends
void handle_sigchld(int sig) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("\n[%d] done\n", pid);
        fflush(stdout);
    }
}

// Main execute function
void execute(char **arglist) {
    signal(SIGCHLD, handle_sigchld);

    // 🔹 Detect background execution
    int background = 0;
    for (int i = 0; arglist[i] != NULL; i++) {
        if (strcmp(arglist[i], "&") == 0) {
            background = 1;
            arglist[i] = NULL; // remove '&' from arguments
            break;
        }
    }

    // 🔹 Detect pipe in the command
    int has_pipe = 0;
    for (int i = 0; arglist[i] != NULL; i++) {
        if (strcmp(arglist[i], "|") == 0) {
            has_pipe = 1;
            break;
        }
    }

    // 🔹 Handle normal or background command (no pipe)
    if (!has_pipe) {
        execute_single(arglist, background);
        return;
    }

    // 🔹 Handle piping
    char **commands[10];  // support up to 10 piped commands
    int num_cmds = split_pipes(arglist, commands);
    int fd[2], in_fd = 0;

    for (int i = 0; i < num_cmds; i++) {
        pipe(fd);
        pid_t pid = fork();

        if (pid == -1) {
            perror("fork failed");
            exit(1);
        }

        if (pid == 0) {
            dup2(in_fd, STDIN_FILENO);
            if (i < num_cmds - 1) {
                dup2(fd[1], STDOUT_FILENO);
            }
            close(fd[0]);
            execvp(commands[i][0], commands[i]);
            perror("Command execution failed");
            exit(1);
        } else {
            if (!background) waitpid(pid, NULL, 0);
            close(fd[1]);
            in_fd = fd[0];
        }
    }

    if (background)
        printf("Pipeline running in background\n");
}

