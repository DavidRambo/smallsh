#include "commands.h"
#include "processes.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// For toggling foreground-only mode.
int fg_only = 0;

void handle_SIGTSTP_fg_on(int signo);
void handle_SIGTSTP_fg_off(int signo);

/*
 * Entry point to the smallsh C program.
 */
int main(void) {
    Command curr_cmd;
    Process procs = NULL;
    struct sigaction SIGINT_action = {0};
    struct sigaction SIGTSTP_action = {0};

    // Register handler to ignore SIGINT.
    SIGINT_action.sa_handler = SIG_IGN;
    // Install the handler.
    sigaction(SIGINT, &SIGINT_action, NULL);

    // Register SIGTSTP handler to turn on foreground-only mode.
    SIGTSTP_action.sa_handler = handle_SIGTSTP_fg_on;
    // Install the handler.
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);

    while (true) {
        procs = check_bg_processes(procs);

        curr_cmd = parse_command(fg_only);

        // parse_command() returns NULL when i/o redirection is followed by
        // command arguments, when the entry is blank, and when it is a comment.
        if (curr_cmd == NULL) {
            continue;
        }

        procs = process_command(curr_cmd, procs);

        free(curr_cmd); // Free memory before parsing another command.
    }

    return EXIT_SUCCESS;
}

/**
 * SIGTSTP handler function to turn on foreground-only mode.
 *
 * Registers its counterpart handler function to turn off foreground-only mode.
 */
void handle_SIGTSTP_fg_on(int signo) {
    struct sigaction SIGTSTP_action = {0};

    // Register SIGTSTP handler to turn on foreground-only mode.
    SIGTSTP_action.sa_handler = handle_SIGTSTP_fg_off;
    // Install the handler.
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);

    write(STDOUT_FILENO, "Entering foreground-only mode (& is now ignored)\n",
          50);
    fg_only = 1;
}

/**
 * SIGTSTP handler function to turn off foreground-only mode.
 *
 * Registers its counterpart handler function to turn on foreground-only mode.
 */
void handle_SIGTSTP_fg_off(int signo) {
    struct sigaction SIGTSTP_action = {0};

    // Register SIGTSTP handler to turn on foreground-only mode.
    SIGTSTP_action.sa_handler = handle_SIGTSTP_fg_on;
    // Install the handler.
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);

    write(STDOUT_FILENO, "Exiting foreground-only mode\n", 30);
    fg_only = 0;
}
