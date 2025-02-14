#include "commands.h"
#include <stdlib.h>

/*
 * Main source code file for the smallsh C program.
 */

int main(void) {
    Command curr_cmd;

    while (true) {
        curr_cmd = parse_command();

#if DEBUG
        int res = print_command(curr_cmd);
        if (res != 0) {
            exit(EXIT_FAILURE);
        }
#endif
    }

    return EXIT_SUCCESS;
}
