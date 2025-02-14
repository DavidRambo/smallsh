#include "commands.h"
#include <stdlib.h>

/*
 * Main source code file for the smallsh C program.
 */

int main(void) {
    Command curr_cmd;

    while (true) {
        curr_cmd = parse_command();

        // parse_command() returns NULL when i/o redirection is followed by
        // command arguments.
        if (curr_cmd == NULL) {
            continue;
        }

#if DEBUG
        int res = print_command(curr_cmd);
        if (res != 0) {
            exit(EXIT_FAILURE);
        }
#endif

        free(curr_cmd); // Free memory before parsing another command.
    }

    return EXIT_SUCCESS;
}
