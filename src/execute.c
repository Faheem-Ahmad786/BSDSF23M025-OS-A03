#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>

#define MAX_JOBS 20

typedef struct {
    pid_t pid;
    char cmdline[100];
    char status[20]; // Running / Stopped / Done
    int id;
} Job;

Job jobs[MAX_JOBS];
int job_count = 0;

// Add new job to list
void add_job(pid_t pid, char *cmdline, char *status) {
    if (job_count < MAX_JOBS) {
        jobs[job_count].pid = pid;
        strcpy(jobs[job_count].cmdline, cmdline);
        strcpy(jobs[job_count].status, status);
        jobs[job_count].id = job_count + 1;
        job_count++;
    }
}

// Remove completed job
void remove_job(pid_t pid) {
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].pid == pid) {
            for (int j = i; j < job_count - 1; j++) {
                jobs[j] = jobs[j + 1];
            }
            job_count--;
            break;
        }
    }
}

// Display all jobs
void list_jobs() {
    for (int i = 0; i < job_count; i++) {
        printf("[%d] %d %s %s\n",
               jobs[i].id,
               jobs[i].pid,
               jobs[i].status,
               jobs[i].cmdline);
    }
}

// Signal handler for finished background processes
void handle_sigchld(int sig) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("\n[%d] done\n", pid);
        fflush(stdout);
        remove_job(pid);
    }
}

// Foreground a background job
void fg_command(int job_id) {
    if (job_id <= 0 || job_id > job_count) {
        printf("Invalid job ID\n");
        return;
    }

    pid_t pid = jobs[job_id - 1].pid;
    printf("Bringing job [%d] (%d) to foreground\n", job_id, pid);
    strcpy(jobs[job_id - 1].status, "Foreground");
    int status;
    waitpid(pid, &status, 0);
    remove_job(pid);
}

// Resume a stopped job in background
void bg_command(int job_id) {
    if (job_id <= 0 || job_id > job_count) {
        printf("Invalid job ID\n");
        return;
    }

    pid_t pid = jobs[job_id - 1].pid;
    printf("Resuming job [%d] (%d) in background\n", job_id, pid);
    kill(pid, SIGCONT);
    strcpy(jobs[job_id - 1].status, "Running");
}

// Helper: execute a normal or background command
void execute_single(char **arglist, int background, char *cmdline) {
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork failed");
        exit(1);
    }

    if (pid == 0) {
        execvp(arglist[0], arglist);
        perror("Command execution failed");
        exit(1);
    } else {
        if (background) {
            printf("[%d] %d running in background\n", job_count + 1, pid);
            add_job(pid, cmdline, "Running");
            return;
        } else {
            waitpid(pid, NULL, 0);
        }
    }
}

// Main execute function
void execute(char **arglist) {
    signal(SIGCHLD, handle_sigchld);

    if (arglist == NULL || arglist[0] == NULL) return;

    // Handle built-in job commands
    if (strcmp(arglist[0], "jobs") == 0) {
        list_jobs();
        return;
    }

    if (strcmp(arglist[0], "fg") == 0 && arglist[1]) {
        fg_command(atoi(arglist[1]));
        return;
    }

    if (strcmp(arglist[0], "bg") == 0 && arglist[1]) {
        bg_command(atoi(arglist[1]));
        return;
    }

    // Detect background execution
    int background = 0;
    for (int i = 0; arglist[i] != NULL; i++) {
        if (strcmp(arglist[i], "&") == 0) {
            background = 1;
            arglist[i] = NULL;
            break;
        }
    }

    // Execute normally or in background
    char cmdline[100] = "";
    for (int i = 0; arglist[i] != NULL; i++) {
        strcat(cmdline, arglist[i]);
        strcat(cmdline, " ");
    }

    execute_single(arglist, background, cmdline);
}

