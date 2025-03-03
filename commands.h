#ifndef COMMANDS_H
#define COMMANDS_H

#include "processes.h"
#include <stdbool.h>

// Suggested directives from sample_parser.c
#define INPUT_LENGTH 2048
#define MAX_ARGS 512

#define PROMPT ": "

// Incomplete type to encapsulate its data structure.
// The struct is implemented in commands.c.
struct command_entry;

// Client code interfaces with the command_entry struct through its pointer.
typedef struct command_entry *Command;

Process background_command(Command cmd, Process procs);
void free_command(Command cmd);
Command parse_command(int fg_only);
int print_command(Command cmd);
Process process_command(Command cmd, Process procs);
int redirect_in(char *infile);
int redirect_out(char *outfile);
void execute_command(Command cmd);

#endif
