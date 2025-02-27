/**
 * Implementation of data structure to track processes running in the
 * background.
 */

#include "processes.h"
#include "builtins.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

void term_proc(pid_t pid);

/**
 * Linked list struct to store information about processes running in the
 * background of smallsh.
 */
struct process {
    pid_t pid;
    struct process *next;
};

/**
 * Creates a new process struct and adds it to the head of the list.
 */
Process add_proc(Process head, pid_t pid) {
    Process new_proc = malloc(sizeof(struct process));
    new_proc->pid = pid;
    new_proc->next = head;

    return new_proc;
}

/**
 * Checks for any terminated background process. If one is found, then prints
 * its pid and status to the console and removes it from the Process list.
 */
Process check_bg_processes(Process head) {
    int wstatus;
    pid_t child_pid;

    child_pid = waitpid(-1, &wstatus, WNOHANG);

    // Print message re terminating background process before prompt.
    if (child_pid > 0) {
        // Update smallsh status with bg process.
        update_status(wstatus);

        printf("background pid %d is done: ", child_pid);
        print_status();

        // Remove process's pid from the Process list.
        head = rm_proc(head, child_pid);
    }

    return head;
}

/**
 * Traverses the list of processes for a matching pid, returning a pointer to
 * the process struct. Returns NULL if not found.
 */
Process find_proc(Process head, pid_t pid) {
    while (head != NULL) {
        if (head->pid == pid) {
            return head;
        }

        head = head->next;
    }

    return NULL;
}

/**
 * Traverses the list of processes, terminating the processes and freeing the
 * memory allocated for their structs.
 */
void kill_all(Process head) {
    while (head != NULL) {
        term_proc(head->pid);
        head = head->next;
    }
}

/**
 * Removes the process with pid from the list if it exists, returning the head
 * of the list.
 */
Process rm_proc(Process head, pid_t pid) {
    int kill_result;

    // Check whether the head is the process.
    if (head->pid == pid) {
        return head->next;
    }

    Process ptr = head;

    while (ptr->next != NULL) {
        if (ptr->next->pid == pid) {
            // Terminate the process.
            term_proc(pid);

            // Remove the process struct from the list.
            Process tmp = ptr->next;
            ptr->next = ptr->next->next;

            // Remove it from the heap.
            free(tmp);

            return head;
        }

        ptr = ptr->next;
    }

    return head;
}

/**
 * Helper function used to terminate a running process. It first attempts a
 * SIGTERM kill command. If unsuccessful, it uses the SIGKILL signal.
 */
void term_proc(pid_t pid) {
    int kill_result;

    kill_result = kill(pid, SIGTERM);

    if (kill_result == -1) {
        kill_result = kill(pid, SIGKILL);

        if (kill_result != 0) {
            perror("kill()");
        }
    }
}
