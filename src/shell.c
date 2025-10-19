#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

// ==================================================
// 🔹 Command History Variables
// ==================================================
#define HISTORY_SIZE 20
char* history[HISTORY_SIZE];
int history_count = 0;

// ==================================================
// 🔹 Function: read_cmd
// Purpose: Display prompt, read user input, store history
// ==================================================
char* read_cmd(char* prompt, FILE* fp) {
    printf("%s", prompt);
    fflush(stdout);

    char* cmdline = (char*) malloc(sizeof(char) * MAX_LEN);
    int c, pos = 0;

    while ((c = getc(fp)) != EOF) {
        if (c == '\n') break;
        cmdline[pos++] = c;
    }

    if (c == EOF && pos == 0) {
        free(cmdline);
        return NULL; // Handle Ctrl+D
    }
    
    cmdline[pos] = '\0';

    // 🔹 Save command to history (skip empty lines)
    if (cmdline[0] != '\0') {
        if (history_count < HISTORY_SIZE) {
            history[history_count++] = strdup(cmdline);
        } else {
            // If full, remove oldest command
            free(history[0]);
            for (int i = 1; i < HISTORY_SIZE; i++) {
                history[i - 1] = history[i];
            }
            history[HISTORY_SIZE - 1] = strdup(cmdline);
        }
    }

    return cmdline;
}

// ==================================================
// 🔹 Function: tokenize
// Purpose: Split command line into tokens
// ==================================================
char** tokenize(char* cmdline) {
    if (cmdline == NULL || cmdline[0] == '\0' || cmdline[0] == '\n') {
        return NULL;
    }

    char** arglist = (char**)malloc(sizeof(char*) * (MAXARGS + 1));
    for (int i = 0; i < MAXARGS + 1; i++) {
        arglist[i] = (char*)malloc(sizeof(char) * ARGLEN);
        bzero(arglist[i], ARGLEN);
    }

    char* cp = cmdline;
    char* start;
    int len;
    int argnum = 0;

    while (*cp != '\0' && argnum < MAXARGS) {
        while (*cp == ' ' || *cp == '\t') cp++;
        if (*cp == '\0') break;

        start = cp;
        len = 1;
        while (*++cp != '\0' && !(*cp == ' ' || *cp == '\t')) {
            len++;
        }
        strncpy(arglist[argnum], start, len);
        arglist[argnum][len] = '\0';
        argnum++;
    }

    if (argnum == 0) {
        for (int i = 0; i < MAXARGS + 1; i++) free(arglist[i]);
        free(arglist);
        return NULL;
    }

    arglist[argnum] = NULL;
    return arglist;
}

// ==================================================
// 🔹 Function: handle_builtin
// Purpose: Execute built-in commands (Feature 2 + history)
// ==================================================
int handle_builtin(char **args) {
    if (args == NULL || args[0] == NULL)
        return 1;

    // exit command
    if (strcmp(args[0], "exit") == 0) {
        printf("Exiting myshell...\n");
        exit(0);
    }

    // cd command
    else if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, "myshell: expected argument to \"cd\"\n");
        } else {
            if (chdir(args[1]) != 0) {
                perror("myshell");
            }
        }
        return 1;
    }

    // help command
    else if (strcmp(args[0], "help") == 0) {
        printf("myshell built-in commands:\n");
        printf("  cd <directory>  - Change directory\n");
        printf("  exit            - Exit the shell\n");
        printf("  help            - Show this help message\n");
        printf("  jobs            - Show background jobs (not implemented yet)\n");
        printf("  history         - Display recent commands\n");
        printf("\nYou can also run external commands like ls, pwd, whoami, etc.\n");
        return 1;
    }

    // jobs command
    else if (strcmp(args[0], "jobs") == 0) {
        printf("Job control not yet implemented.\n");
        return 1;
    }

    // history command
    else if (strcmp(args[0], "history") == 0) {
        for (int i = 0; i < history_count; i++) {
            printf("%d  %s\n", i + 1, history[i]);
        }
        return 1;
    }

    return 0; // Not a built-in command
}

