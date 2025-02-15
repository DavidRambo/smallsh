#ifndef BUILTINS_H
#define BUILTINS_H

#include "commands.h"

// Stub for status code struct. Implementation is in builtins.c.
struct status;
typedef struct status Status;

void change_directory(char *argv[], int argc);
void print_status(void);
void set_status(int kind, int new_status);

#endif
