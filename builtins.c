#include "builtins.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Tracks the status of the last process to terminate.
struct status {
    enum { EXIT_CODE = 0, SIGNAL = 1 } kind;
    int code;
};

// External variable to track status while smallsh is running.
Status status = {0, 0};

/*
 * Changes the current working directory of smallsh using the
 *
 * With no arguments, sets the pwd to the user's $HOME.
 * Takes an optional argument, which may be a relative or absolute pathname.
 */
void change_directory(char *argv[], int argc) {
    int result;

    // Check for correct number of arguments.
    if (argc > 2) {
        printf("smallsh: cd: too many arguments\n");
        return;
    }

    // Check for argument.
    if (argc == 1) {
        // Go $HOME.
        char *home_path = getenv("HOME");
        if (home_path == NULL) {
            printf("No directory path set for user's $HOME.\n");
            return;
        }
        // Set global g_curr_dir variable to user's $HOME
        result = chdir(home_path);
        if (result != 0) {
            perror("chdir() to HOME");
            return;
        }
    } else {
        // Try to change working directory to first command argument.
        result = chdir(argv[1]);

        // Check that the path was a valid directory.
        if (result != 0) {
            perror("chdir()");
        }
    }

#if DEBUG
    char *curr_dir = getcwd(NULL, 512);
    printf(">>> change_directory() -> pwd is %s\n", curr_dir);
    free(curr_dir);
#endif
}

void print_status(void) {
    switch (status.kind) {
        case EXIT_CODE:
            printf("exit value %d\n", status.code);
            break;
        case SIGNAL:
            printf("terminated by signal %d\n", status.code);
            break;
        default:
            printf("Error: invalid status code.\n");
            break;
    }
}

void set_status(int kind, int new_status) {
    status.kind = kind;
    status.code = new_status;
}
