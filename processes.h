#ifndef PROCESSES_H
#define PROCESSES_H

#include <sys/types.h>

// Stub for process struct, which is implemented in processes.c.
struct process;

typedef struct process *Process;

Process add_proc(Process head, pid_t pid);
Process find_proc(Process head, pid_t pid);
void kill_all(Process head);
Process rm_proc(Process head, pid_t pid);

#endif
