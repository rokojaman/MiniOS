// process.c 

#include "process.h"
#include "memory.h"

// External functions from kernel
extern void print(const char* str);
extern void print_dec(unsigned int n);
extern void putchar(char c);

// Process table
static struct pcb process_table[MAX_PROCESSES];
static int next_pid = 1;

// Current running process
struct pcb* current_process = 0;

// Initialize process management
void process_init() {
    print("Process manager: Starting init (from process.c)\n");
    print("Process manager: Initializing...\n");
    
    // Clear process table
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i].pid = 0;
        process_table[i].state = PROCESS_ZOMBIE;
        process_table[i].esp = 0;
        process_table[i].ebp = 0;
        process_table[i].eip = 0;
        process_table[i].cr3 = 0;
        process_table[i].stack_base = 0;
        // Clear name
        for (int j = 0; j < 32; j++) {
            process_table[i].name[j] = '\0';
        }
    }
    
    // Create idle process (PID 0) - this is our kernel/shell
    process_table[0].pid = 0;
    process_table[0].state = PROCESS_RUNNING;
    process_table[0].esp = 0;
    process_table[0].ebp = 0;
    process_table[0].eip = 0;
    
    // Copy "kernel" to name
    const char* kernel_name = "kernel";
    for (int i = 0; i < 7; i++) {
        process_table[0].name[i] = kernel_name[i];
    }
    
    current_process = &process_table[0];
    
    print("Process manager: Initialized\n");
}

// Create a new process (simplified - no actual execution yet)
int process_create(const char* name, void (*entry_point)()) {
    // Find free slot
    int slot = -1;
    for (int i = 1; i < MAX_PROCESSES; i++) {
        if (process_table[i].state == PROCESS_ZOMBIE) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        print("Process manager: No free slots\n");
        return -1;
    }
    
    // Initialize PCB
    process_table[slot].pid = next_pid++;
    process_table[slot].state = PROCESS_READY;
    process_table[slot].eip = (unsigned int)entry_point;
    
    // Copy name
    int i;
    for (i = 0; i < 31 && name[i]; i++) {
        process_table[slot].name[i] = name[i];
    }
    process_table[slot].name[i] = '\0';
    
    print("Process created: ");
    print(name);
    print(" (PID ");
    print_dec(process_table[slot].pid);
    print(")\n");
    
    return process_table[slot].pid;
}

// List all processes
void process_list() {
    print("TEST: process_list function called!\n");
}
