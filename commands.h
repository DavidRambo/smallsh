#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdbool.h>

// Suggested directives from sample_parser.c
#define INPUT_LENGTH 2048
#define MAX_ARGS 512

#define PROMPT ": "

// Incomplete type to encapculate its data structure.
// The struct is implemented in the C source code file.
struct command_entry;

// Client code interfaces with the command_entry struct through its pointer.
typedef struct command_entry *Command;

Command parse_command(void);
int print_command(Command);

#endif
