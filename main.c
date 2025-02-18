#include "commands.h"
#include "processes.h"
#include <signal.h>
#include <stdlib.h>

/*
 * Entry point to the smallsh C program.
 */
int main(void) {
    Command curr_cmd;
    Process procs = NULL;
    struct sigaction SIGINT_action = {0};

    // Register handler to ignore SIGINT.
    SIGINT_action.sa_handler = SIG_IGN;
    // Install the handler.
    sigaction(SIGINT, &SIGINT_action, NULL);

    while (true) {
        procs = check_bg_processes(procs);

        curr_cmd = parse_command();

        // parse_command() returns NULL when i/o redirection is followed by
        // command arguments, when the entry is blank, and when it is a comment.
        if (curr_cmd == NULL) {
            continue;
        }

#if DEBUG
        int res = print_command(curr_cmd);
        if (res != 0) {
            exit(EXIT_FAILURE);
        }
#endif

        procs = process_command(curr_cmd, procs);

        free(curr_cmd); // Free memory before parsing another command.
    }

    return EXIT_SUCCESS;
}
