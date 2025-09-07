// process.h 

#ifndef PROCESS_H
#define PROCESS_H

// Process states
#define PROCESS_READY    0
#define PROCESS_RUNNING  1
#define PROCESS_BLOCKED  2
#define PROCESS_ZOMBIE   3

// Maximum processes
#define MAX_PROCESSES    8

// Process Control Block
struct pcb {
    unsigned int pid;           // Process ID
    unsigned int state;         // Process state
    unsigned int esp;           // Stack pointer
    unsigned int ebp;           // Base pointer
    unsigned int eip;           // Instruction pointer
    unsigned int cr3;           // Page directory (not used yet)
    char name[32];             // Process name
    unsigned int stack_base;    // Stack base address
};

// Process management functions
void process_init();
int process_create(const char* name, void (*entry_point)());
void process_yield();
void process_exit();
void process_list();

// Global current process
extern struct pcb* current_process;

#endif
