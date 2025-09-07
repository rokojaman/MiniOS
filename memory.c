// memory.c

#include "memory.h"

// External function from kernel
extern void print(const char* str);
extern void putchar(char c);
extern void print_dec(unsigned int n);

// Track if memory is initialized
static int memory_initialized = 0;

// Define a fixed memory region (start at 2MB, size 1MB)
#define MEMORY_START 0x200000
#define MEMORY_SIZE  0x100000  // 1MB
#define MEMORY_END   (MEMORY_START + MEMORY_SIZE)

// Very simple allocator - just track next free address
static unsigned int next_free_addr = MEMORY_START;

// Track the file system allocation separately
static unsigned int fs_allocation_start = 0;
static unsigned int fs_allocation_size = 0;

// Initialize memory - does almost nothing
void memory_init() {
    memory_initialized = 1;
    next_free_addr = MEMORY_START;
    fs_allocation_start = 0;
    fs_allocation_size = 0;
}

// Very simple malloc - just bump the pointer
void* malloc(unsigned int size) {
    // Check if initialized
    if (!memory_initialized) {
        return 0;
    }
    
    // Check for space
    if (next_free_addr + size > MEMORY_END) {
        return 0;  // Out of memory
    }
    
    // Get current address
    void* result = (void*)next_free_addr;
    
    // Move pointer forward
    next_free_addr += size;
    
    // Align to 4 bytes
    next_free_addr = (next_free_addr + 3) & ~3;
    
    return result;
}

// Special function for file system to register its allocation
void memory_register_fs(void* addr, unsigned int size) {
    fs_allocation_start = (unsigned int)addr;
    fs_allocation_size = size;
}

// Print memory statistics
void memory_free() {
    if (!memory_initialized) {
        return;
    }
    
    unsigned int free = (MEMORY_END - next_free_addr) / 1024;
    
    print_dec(free);
}

void memory_used() {
    if (!memory_initialized) {
        return;
    }

    unsigned int used = next_free_addr - MEMORY_START;

    print_dec(used);
}

// Free all allocated memory EXCEPT file system (reset allocator)
void free_all() {
    if (!memory_initialized) {
        return;
    }
    
    // If file system is allocated, reset to just after it
    if (fs_allocation_size > 0) {
        next_free_addr = fs_allocation_start + fs_allocation_size;
        // Align to 4 bytes
        next_free_addr = (next_free_addr + 3) & ~3;
    } else {
        // No file system, reset to start
        next_free_addr = MEMORY_START;
    }
}
