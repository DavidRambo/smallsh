#include "commands.h"
#include <stdlib.h>

/*
 * Main source code file for the smallsh C program.
 */

int main(void) {
    Command curr_cmd;

    while (true) {
        curr_cmd = parse_command();

    return EXIT_SUCCESS;
}
