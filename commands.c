#include "commands.h"
#include "builtins.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/*
 * Represents a parsed smallsh command entered at the prompt.
 *
 * Adapted from sample_parser.c
 *
 * Fields:
 * argv : array of char* pointers, one for each argument
 * argc : the count of command arguments
 * in_file : name of a file from which to read input
 * out_file : name of a file from to which to write output
 * is_bg : whether to run the command as a background process
 *
 * Entered commands may be accessed in order via
 * command_entry.argv[command_entry.argc].
 *
 */
struct command_entry {
    char *argv[MAX_ARGS + 1];
    int argc;
    char *in_file;
    char *out_file;
    bool is_bg;
};

/*
 * Parses a smallsh command entered at the prompt.
 *
 * This parse command is adapted from sample_parse.c
 *
 * The prompt is a colon, and the syntax for a command is:
 *  : command [arg1 arg2 arg3 ...] [< input_filename] [> output_filename] [&]
 *
 * The concluding ampersand is for running a command as a background process.
 * It must be the last character of a command, else it is interpreted as text.
 *
 * Input is retrieved from stdin, set to a maximum of INPUT_LENGTH characters.
 * It is tokenized at spaces and a terminating newline.
 */
Command parse_command(void) {
    char input[INPUT_LENGTH];
    Command cmd = (Command)calloc(1, sizeof(struct command_entry));

    // To ensure that i/o redirection occurs after command and arguments.
    int args_done = 0;

    // Print prompt.
    printf(PROMPT);
    fflush(stdout);
    fgets(input, INPUT_LENGTH, stdin);

    // Tokenize input into commands.
    char *cmd_tok_ptr;
    char *token = strtok_r(input, " \n", &cmd_tok_ptr);

    // Check for blank line.
    if (token == NULL) {
        return NULL;
    }

    // Check for comment, which begins with a hash.
    if (strncmp(token, "#", 1) == 0) {
        return NULL;
    }

    while (token != NULL) {
        if (strcmp(token, "<") == 0) {
            // Redirect stdin.
            cmd->in_file = strdup(strtok_r(NULL, " \n", &cmd_tok_ptr));
            args_done = 1;
        } else if (strcmp(token, ">") == 0) {
            // Redirect stdout.
            cmd->out_file = strdup(strtok_r(NULL, " \n", &cmd_tok_ptr));
            args_done = 1;
        } else if (strcmp(token, "&") == 0) {
            // Run as background job.
            cmd->is_bg = true;
        } else if (!args_done) {
            // Add to list of arguments.
            cmd->argv[cmd->argc++] = strdup(token);
            // cmd->argc tracks the cmd->argv subscript for the current token,
            // incrementing so that it reflects the total number of tokens
            // stored.
        } else {
            // More command arguments were received after redirection.
            printf("Error: command arguments must precede input/output "
                   "redirection.\n");
            fflush(stdout);
            return NULL;
        }

        token = strtok_r(NULL, " \n", &cmd_tok_ptr);
    }

    return cmd;
}

#if DEBUG
/*
 * For debugging purposes, this function prints out the parsed command.
 */
int print_command(Command cmd) {
    if (cmd == NULL) {
        printf("Error, the command is NULL.\n");
        return EXIT_FAILURE;
    }

    if (cmd->argc == 0) {
        printf("No commands entered.\n");
        return EXIT_SUCCESS;
    } else {
        printf("Command = %s\n", cmd->argv[0]);
        for (int c = 1; c < cmd->argc; c++) {
            printf("Arg %d : %s\n", c, cmd->argv[c]);
        }
    }

    if (cmd->in_file != NULL) {
        printf("Redirect input from : %s\n", cmd->in_file);
    }
    if (cmd->out_file != NULL) {
        printf("Redirect output to : %s\n", cmd->out_file);
    }

    return EXIT_SUCCESS;
}
#endif

/*
 * Dispatcher function for running a parsed command.
 *
 * First checks whether the command is one of smallsh's three built-ins:
 *  - exit : exits the shell, killing any processes or jobs it has started
 *  - cd : changes the working directory, using absolute or relative paths
 *  - status : prints either the exit status or the terminating signal of the
 *      last foreground process run by smallsh
 *
 *  No i/o redirection, background argument is ignored, no exit status is set.
 *
 *  If the command is not a built-in, then it sends the command to a generic
 *  execution function.
 */
void process_command(Command cmd) {
    // Check for built-in commands.
    if (strcmp(cmd->argv[0], "exit") == 0) {
        // TODO: Kill running processes and jobs.
        // kill_all();
        exit(EXIT_SUCCESS);
    } else if (strcmp(cmd->argv[0], "cd") == 0) {
        change_directory(cmd->argv, cmd->argc);
    } else if (strcmp(cmd->argv[0], "status") == 0) {
        // Display status of last foreground process via stdout.
        print_status();
    } else {
        // Not a built-in, so fork a child process to run the command.
        // This switch statement idea is from Dr. Guillermo Tonsmann's
        // "Processes" pdf, p.30.
        pid_t spawn_pid, child_pid;
        int result;
        switch (spawn_pid = fork()) {
            case -1:
                perror("fork() failed");
                exit(EXIT_FAILURE);

            case 0: // Child process.
                // Append a NULL to the array of args for the execvp call.
                cmd->argv[cmd->argc] = NULL;

                // The child process executes the command.
                execvp(cmd->argv[0], cmd->argv);

                perror("execvp()");
                exit(EXIT_FAILURE);

                break;

            default:
#if DEBUG
                printf("In parent process about to wait on child process %d\n",
                       spawn_pid);
                fflush(stdout);
#endif
                // The parent process waits for the child process to terminate.
                child_pid = waitpid(spawn_pid, &result, 0);
#if DEBUG
                printf("Parent done waiting. waitpid returned %d\n", child_pid);
                fflush(stdout);
#endif

                // Update smallsh's Status.
                update_status(result);

                break;
        }
    }
}
