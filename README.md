# smallsh

A lil shell for OSU CS 374.

To compile, run `make smallsh` and then execute the binary `./smallsh`.

## Usage

### Command Syntax

A `smallsh` command has the form

```
: command [arg1] [arg2] [...] [< input_file] [> output_file] [&]
: # This is a comment.
```

The ampersand must come at the end of the command in order to be treated as a background process.

### Built-In Commands

`smallsh` has three built-in commands:

- `exit` exits the shell.
- `cd` changes the working directory
- `status` prints out the status of the most recently terminated command

### Other commands

`smallsh` will run arbitrary commands accessible in the host system's PATH.

### Foreground-only Mode

Pressing Ctrl-z turns on "foreground-only mode," during which `smallsh` ignores the appended ampersands on commands.
Pressing Ctrl-z again turns it off.

Bug: Note that pressing Ctrl-z while a process is running in the foreground will crash `smallsh` once the process returns.
I tried temporarily blocking `SIGTSTP` while the parent process waits for the child process to terminate, but code adapted from Kerrisk's _TLPI_ did not work correctly.
