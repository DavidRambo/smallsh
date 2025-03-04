#include "commands.h"
#include "builtins.h"
#include "processes.h"
#include <fcntl.h>
#include <signal.h>
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
 * Or, via struct command_entry* pointer: cmd->argv[cmg->argc].
 *
 */
struct command_entry {
    char *argv[MAX_ARGS + 1];
    int argc;
    char *in_file;
    char *out_file;
    bool is_bg;
};

void handle_fg_SIGINT(int signo) {
    _exit(EXIT_SUCCESS);
}

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
Command parse_command(int fg_only) {
    char input[INPUT_LENGTH] = {0};
    Command cmd = (Command)calloc(1, sizeof(struct command_entry));

    // To ensure that i/o redirection occurs after command and arguments.
    int args_done = 0;

    // Print prompt.
    printf(PROMPT);
    fflush(stdout);

    // Get input from user.
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
            if (!fg_only) {
                // Run as background job unless foreground-only mode is on.
                cmd->is_bg = true;
            }
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
Process process_command(Command cmd, Process procs) {
    // Check for built-in commands.
    if (strcmp(cmd->argv[0], "exit") == 0) {
        // Terminate all running processes and jobs.
        kill_all(procs);

        exit(EXIT_SUCCESS);
    } else if (strcmp(cmd->argv[0], "cd") == 0) {
        change_directory(cmd->argv, cmd->argc);
    } else if (strcmp(cmd->argv[0], "status") == 0) {
        // Display status of last foreground process via stdout.
        print_status();
    } else if (cmd->is_bg) {
        // Process is set to run in the background.
        procs = background_command(cmd, procs);
    } else {
        // Not a built-in, so fork a child process to run the command.
        execute_command(cmd);
    }
    return procs;
}

void execute_command(Command cmd) {
    pid_t spawn_pid, child_pid;
    int result;

    // This switch statement idea is from Dr. Guillermo Tonsmann's
    // "Processes" pdf, p.30.
    switch (spawn_pid = fork()) {
        case -1:
            perror("fork() failed");
            exit(EXIT_FAILURE);

        case 0: // Child process.
            if (cmd->in_file != NULL) {
                result = redirect_in(cmd->in_file);
                if (result) {
                    _exit(EXIT_FAILURE);
                    // parent process takes care of updating status
                }
            }

            if (cmd->out_file != NULL) {
                result = redirect_out(cmd->out_file);
                if (result) {
                    _exit(EXIT_FAILURE);
                    // parent process takes care of updating status
                }
            }

            // Append a NULL to the array of args for the execvp call.
            cmd->argv[cmd->argc] = NULL;

            struct sigaction SIGINT_action = {0};
            struct sigaction SIGTSTP_action = {0};

            /* These lines are adapted from "Exploration: Signal Handling API".
             * https://canvas.oregonstate.edu/courses/1987883/pages/exploration-signal-handling-api?module_item_id=24956227
             * 2025-02-18
             */
            // Register handler for SIGINT.
            // This will not apply to exec() commands.
            SIGINT_action.sa_handler = handle_fg_SIGINT;
            // Block catchable signals while SIGINT is running.
            sigfillset(&SIGINT_action.sa_mask);
            // No flags.
            SIGINT_action.sa_flags = 0;
            // Install the handler.
            sigaction(SIGINT, &SIGINT_action, NULL);

            // Register handler to ignore SIGTSTP.
            // This will apply to exec() commands.
            SIGTSTP_action.sa_handler = SIG_IGN;
            // Install the handler.
            sigaction(SIGTSTP, &SIGTSTP_action, NULL);

            // The child process executes the command.
            execvp(cmd->argv[0], cmd->argv);

            perror("execvp()");
            _exit(EXIT_FAILURE);
            // parent process takes care of updating status

            break;

        default:
            // The parent process waits for the child process to terminate.
            child_pid = waitpid(spawn_pid, &result, 0);

            // Update smallsh's Status.
            update_status(result);

            if (WIFSIGNALED(result)) {
                print_status();
            }

            break;
    }
}

Process background_command(Command cmd, Process procs) {
    pid_t spawn_pid;
    int result;

    switch (spawn_pid = fork()) {
        case -1:
            perror("fork() failed");
            exit(EXIT_FAILURE);

        case 0: // Child process.
            // Must redirect i/o.
            if (cmd->in_file == NULL) {
                result = redirect_in("/dev/null");
            } else {
                result = redirect_in(cmd->in_file);
            }
            if (result) {
                _exit(EXIT_FAILURE);
            }

            if (cmd->out_file == NULL) {
                result = redirect_out("/dev/null");
            } else {
                result = redirect_out(cmd->out_file);
            }
            if (result) {
                _exit(EXIT_FAILURE);
            }

            // Set background process to ignore SIGINT and SIGTSTP signals.
            // Ignored signals apply to commands called with exec().
            struct sigaction SIGINT_action = {0};
            struct sigaction SIGTSTP_action = {0};

            // Register handler to ignore SIGINT.
            SIGINT_action.sa_handler = SIG_IGN;
            // Install the handler.
            sigaction(SIGINT, &SIGINT_action, NULL);

            // Register handler to ignore SIGTSTP.
            SIGTSTP_action.sa_handler = SIG_IGN;
            // Install the handler.
            sigaction(SIGTSTP, &SIGTSTP_action, NULL);

            // Append a NULL to the array of args for the execvp call.
            cmd->argv[cmd->argc] = NULL;

            // The child process executes the command.
            execvp(cmd->argv[0], cmd->argv);

            perror("execvp()");
            _exit(EXIT_FAILURE);
            // parent process takes care of updating status

            break;

        default: // Parent process.
            // Save process in list so that it may be terminated upon smallsh
            // exit.
            procs = add_proc(procs, spawn_pid);

            // Print the PID of the background process when it begins.
            printf("background pid is %d\n", spawn_pid);
            fflush(stdout);

            // Return to the prompt.
            break;
    }

    return procs;
}

/**
 * Redirects stdin file descriptor to the specified pathname.
 *
 * This function prints any errors encountered.
 *
 * Returns 0 if successful, 1 if not.
 */
int redirect_in(char *infile) {
    int newfd;

    // Open file to read from for stdin redirection.
    newfd = open(infile, O_RDONLY);
    if (newfd == -1) {
        printf("cannot open %s for input\n", infile);
        return 1;
    }

    // Redirect stdin to infile's fd.
    newfd = dup2(newfd, STDIN_FILENO);
    if (newfd == -1) {
        perror("dup2");
        return 1;
    }

    return 0;
}

/**
 * Redirects stdout file descriptor to the specified pathname.
 *
 * This function prints any errors encountered.
 *
 * Returns 0 if successful, 1 if not.
 */
int redirect_out(char *outfile) {
    int newfd;

    // Open file to read from for stdin redirection.
    newfd = open(outfile, O_CREAT | O_TRUNC | O_WRONLY, 0640);
    if (newfd == -1) {
        printf("cannot open %s for output\n", outfile);
        return 1;
    }

    // Redirect stdin to infile's fd.
    newfd = dup2(newfd, STDOUT_FILENO);
    if (newfd == -1) {
        perror("dup2");
        return 1;
    }

    return 0;
}

void free_command(Command cmd) {
    for (int argc = 0; argc < cmd->argc; argc++) {
        free(cmd->argv[argc]);
    }
    free(cmd->in_file);
    free(cmd->out_file);
    free(cmd);
}
