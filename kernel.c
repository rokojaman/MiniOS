// kernel.c

#include "idt.h"
#include "keyboard.h"
#include "memory.h"
#include "fs.h"

// VGA text mode constants
#define VGA_ADDRESS 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define WHITE_ON_BLACK 0x07

// VGA I/O ports
#define VGA_CTRL_REGISTER 0x3D4
#define VGA_DATA_REGISTER 0x3D5

// Current cursor position
static unsigned int cursor_x = 0;
static unsigned int cursor_y = 2;  // Start below bootloader message

// Track prompt position to prevent backspace from going too far
static unsigned int prompt_x = 0;
static unsigned int prompt_y = 0;

// Forward declarations
void print(const char* str);
void print_dec(unsigned int n);
void print_hex(unsigned int n);
void putchar(char c);
void clear_screen();
void update_cursor();
void enable_cursor();
void scroll_screen();
void process_command(const char* cmd);
void run_shell();

// Export timer handler for IDT
void timer_handler();

// Process management (integrated into kernel for now)
#define MAX_PROCESSES 8
#define PROCESS_READY    0
#define PROCESS_RUNNING  1
#define PROCESS_BLOCKED  2
#define PROCESS_ZOMBIE   3

struct pcb {
    unsigned int pid;
    unsigned int state;
    char name[32];
};

static struct pcb process_table[MAX_PROCESSES];
static int next_pid = 1;
struct pcb* current_process = 0;

// Initialize process management
void init_processes() {
    
    // Clear process table
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i].pid = 0;
        process_table[i].state = PROCESS_ZOMBIE;
        for (int j = 0; j < 32; j++) {
            process_table[i].name[j] = '\0';
        }
    }
    
    // Create kernel process
    process_table[0].pid = 0;
    process_table[0].state = PROCESS_RUNNING;
    const char* kname = "kernel";
    for (int i = 0; kname[i] && i < 31; i++) {
        process_table[0].name[i] = kname[i];
    }
    
    current_process = &process_table[0];
    print("Process manager initialized\n");
}

// Create a new process
int create_process(const char* name) {
    // Find free slot
    int slot = -1;
    for (int i = 1; i < MAX_PROCESSES; i++) {
        if (process_table[i].state == PROCESS_ZOMBIE) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        print("No free process slots\n");
        return -1;
    }
    
    // Initialize PCB
    process_table[slot].pid = next_pid++;
    process_table[slot].state = PROCESS_READY;
    
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

// Simple round-robin scheduler
void schedule() {
    // Find next ready process
    int current_pid = current_process->pid;
    int next_slot = -1;
    
    // Look for next ready process after current
    for (int i = current_pid + 1; i < MAX_PROCESSES; i++) {
        if (process_table[i].state == PROCESS_READY) {
            next_slot = i;
            break;
        }
    }
    
    // If not found, wrap around
    if (next_slot == -1) {
        for (int i = 0; i <= current_pid; i++) {
            if (process_table[i].state == PROCESS_READY || 
                process_table[i].state == PROCESS_RUNNING) {
                next_slot = i;
                break;
            }
        }
    }
    
    // Switch process (simplified - no actual context switch)
    if (next_slot != -1 && next_slot != current_pid) {
        if (current_process->state == PROCESS_RUNNING) {
            current_process->state = PROCESS_READY;
        }
        current_process = &process_table[next_slot];
        current_process->state = PROCESS_RUNNING;
    }
}

// Timer tick counter
static unsigned int timer_ticks = 0;

// Timer handler
void timer_handler() {
    timer_ticks++;
    
    // Schedule every 30 ticks (about 1 second)
    if (timer_ticks % 30 == 0) {
        schedule();
    }
}

// List processes
void list_processes() {
    print("PID  STATE    NAME\n");
    print("---  -------  ----------------\n");
    
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state != PROCESS_ZOMBIE) {
            // Print PID
            if (process_table[i].pid < 10) print(" ");
            print_dec(process_table[i].pid);
            print("   ");
            
            // Print state
            switch (process_table[i].state) {
                case PROCESS_READY:
                    print("READY   ");
                    break;
                case PROCESS_RUNNING:
                    print("RUNNING ");
                    break;
                case PROCESS_BLOCKED:
                    print("BLOCKED ");
                    break;
                default:
                    print("UNKNOWN ");
            }
            
            // Print name
            print(" ");
            print(process_table[i].name);
            print("\n");
        }
    }
}

// Update hardware cursor position
void update_cursor() {
    unsigned short position = cursor_y * VGA_WIDTH + cursor_x;
    
    // Tell VGA board the high cursor byte is set
    outb(VGA_CTRL_REGISTER, 14);
    outb(VGA_DATA_REGISTER, position >> 8);
    
    // Tell VGA board the low cursor byte is set
    outb(VGA_CTRL_REGISTER, 15);
    outb(VGA_DATA_REGISTER, position & 0xFF);
}

// Enable the cursor
void enable_cursor() {
    // Set cursor start scanline to 14 and end to 15 (block cursor)
    outb(VGA_CTRL_REGISTER, 0x0A);
    outb(VGA_DATA_REGISTER, 14);
    
    outb(VGA_CTRL_REGISTER, 0x0B);
    outb(VGA_DATA_REGISTER, 15);
    
    update_cursor();
}

// Scroll the screen up by one line
void scroll_screen() {
    unsigned short* vga_buffer = (unsigned short*)VGA_ADDRESS;
    
    // Move all lines up by one
    for (int y = 0; y < VGA_HEIGHT - 1; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = vga_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }
    
    // Clear the last line
    for (int x = 0; x < VGA_WIDTH; x++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = (WHITE_ON_BLACK << 8) | ' ';
    }
}

// Function to write a character to the screen
void putchar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
        if (cursor_y >= VGA_HEIGHT) {
            cursor_y = VGA_HEIGHT - 1;
            scroll_screen();
            // Update prompt position after scroll
            if (prompt_y > 0) {
                prompt_y--;
            }
        }
        update_cursor();
        return;
    }
    
    if (c == '\b') {
        // Handle backspace
        if (cursor_x > 0) {
            cursor_x--;
        } else if (cursor_y > 0) {
            cursor_y--;
            cursor_x = VGA_WIDTH - 1;
        }
        // Clear the character at the new position
        unsigned short* vga_buffer = (unsigned short*)VGA_ADDRESS;
        unsigned int index = cursor_y * VGA_WIDTH + cursor_x;
        vga_buffer[index] = (WHITE_ON_BLACK << 8) | ' ';
        update_cursor();
        return;
    }
    
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
        if (cursor_y >= VGA_HEIGHT) {
            cursor_y = VGA_HEIGHT - 1;
            scroll_screen();
            // Update prompt position after scroll
            if (prompt_y > 0) {
                prompt_y--;
            }
        }
    }
    
    unsigned short* vga_buffer = (unsigned short*)VGA_ADDRESS;
    unsigned int index = cursor_y * VGA_WIDTH + cursor_x;
    vga_buffer[index] = (WHITE_ON_BLACK << 8) | c;
    cursor_x++;
    update_cursor();
}

// Function to print a string
void print(const char* str) {
    while (*str) {
        putchar(*str);
        str++;
    }
}

// Clear the screen
void clear_screen() {
    unsigned short* vga_buffer = (unsigned short*)VGA_ADDRESS;
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = (WHITE_ON_BLACK << 8) | ' ';
    }
    cursor_x = 0;
    cursor_y = 0;
    update_cursor();
}

// Helper to print decimal numbers
void print_dec(unsigned int n) {
    if (n == 0) {
        putchar('0');
        return;
    }
    
    char buffer[11];
    int i = 0;
    
    while (n > 0) {
        buffer[i++] = '0' + (n % 10);
        n /= 10;
    }
    
    while (i > 0) {
        putchar(buffer[--i]);
    }
}

// Helper to print hex numbers
void print_hex(unsigned int n) {
    print("0x");
    char hex_chars[] = "0123456789ABCDEF";
    for (int i = 7; i >= 0; i--) {
        putchar(hex_chars[(n >> (i * 4)) & 0xF]);
    }
}

// Command buffer and processing
#define CMD_BUFFER_SIZE 256
#define MAX_FILENAME_LENGTH 12  // Same as in fs.h
#define FILE_SIZE 512          // Same as in fs.h
static char command_buffer[CMD_BUFFER_SIZE];
static int cmd_index = 0;

// External functions from memory
extern void memory_free();
extern void memory_used();
extern void free_all();

// External functions from fs
extern void fs_list_files();
extern int fs_create_file(const char* name);
extern int fs_write_file(const char* name, const unsigned char* data, unsigned int size);
extern int fs_read_file(const char* name, unsigned char* buffer, unsigned int size);
extern int fs_delete_file(const char* name);

// Wrapper to show memory stats
void show_mem_stats() {
    print("Memory Statistics:\n");
    print("  Total: 1024 KB\n");
    print("  Used: ");
    memory_used();  // This will print "used,free"
    print(" bytes\n");
    print("  Free: ");
    memory_free();
    print(" KB\n");
}

// Process a command
void process_command(const char* cmd) {
    if (cmd[0] == '\0') {
        return;  // Empty command
    }
    
    // Simple command parsing
    if (cmd[0] == 'h' && cmd[1] == 'e' && cmd[2] == 'l' && cmd[3] == 'p' && cmd[4] == '\0') {
        print("Available commands:\n");
        print("  help     - Show this help message\n");
        print("  clear    - Clear the screen\n");
        print("  about    - Show system information\n");
        print("  echo     - Echo text back\n");
        print("  mem      - Show memory statistics\n");
        print("  memtest  - Test memory allocation\n");
        print("  memfree  - Free all allocated memory\n");
        print("  ps       - List running processes\n");
        print("  run      - Create a test process\n");
        print("  ls       - List files\n");
        print("  create   - Create a file (usage: create filename)\n");
        print("  write    - Write to file (usage: write filename text)\n");
        print("  read     - Read from file (usage: read filename)\n");
        print("  delete   - Delete a file (usage: delete filename)\n");
    } else if (cmd[0] == 'c' && cmd[1] == 'l' && cmd[2] == 'e' && cmd[3] == 'a' && cmd[4] == 'r' && cmd[5] == '\0') {
        clear_screen();
    } else if (cmd[0] == 'a' && cmd[1] == 'b' && cmd[2] == 'o' && cmd[3] == 'u' && cmd[4] == 't' && cmd[5] == '\0') {
        print("MiniOS v0.1\n");
        print("A simple operating system for educational purposes\n");
        print("Features:\n");
        print("- 32-bit protected mode\n");
        print("- Interrupt handling\n");
        print("- Keyboard input\n");
        print("- Memory management\n");
        print("- Process management\n");
        print("- Basic command shell\n");
    } else if (cmd[0] == 'e' && cmd[1] == 'c' && cmd[2] == 'h' && cmd[3] == 'o' && cmd[4] == ' ') {
        print(&cmd[5]);
        print("\n");
    } else if (cmd[0] == 'm' && cmd[1] == 'e' && cmd[2] == 'm' && cmd[3] == '\0') {
            show_mem_stats();

    } else if (cmd[0] == 'm' && cmd[1] == 'e' && cmd[2] == 'm' && cmd[3] == 't' && cmd[4] == 'e' && cmd[5] == 's' && cmd[6] == 't' && cmd[7] == '\0') {
        // Simple memory test
        print("Testing memory allocation...\n");
        
        void* p1 = malloc(100);
        if (p1) {
            print("Allocated 100 bytes - OK\n");
            
            // Test writing and reading
            unsigned char* bytes = (unsigned char*)p1;
            print("Writing test pattern...\n");
            for (int i = 0; i < 100; i++) {
                bytes[i] = i & 0xFF;
            }
            
            print("Verifying test pattern...\n");
            int ok = 1;
            for (int i = 0; i < 100; i++) {
                if (bytes[i] != (i & 0xFF)) {
                    ok = 0;
                    break;
                }
            }
            
            if (ok) {
                print("Memory read/write test PASSED\n");
            } else {
                print("Memory read/write test FAILED\n");
            }
        } else {
            print("Allocation failed!\n");
        }
        
        void* p2 = malloc(200);
        if (p2) {
            print("Allocated 200 bytes - OK\n");
        } else {
            print("Allocation failed!\n");
        }
        
        
    } else if (cmd[0] == 'm' && cmd[1] == 'e' && cmd[2] == 'm' && cmd[3] == 'f' && cmd[4] == 'r' && cmd[5] == 'e' && cmd[6] == 'e' && cmd[7] == '\0') {
        free_all();
        print("All memory freed\n");
    } else if (cmd[0] == 'p' && cmd[1] == 's' && cmd[2] == '\0') {
        list_processes();
    } else if (cmd[0] == 'r' && cmd[1] == 'u' && cmd[2] == 'n' && cmd[3] == '\0') {
        create_process("test_process");
    } else if (cmd[0] == 'l' && cmd[1] == 's' && cmd[2] == '\0') {
        fs_list_files();
    } else if (cmd[0] == 'c' && cmd[1] == 'r' && cmd[2] == 'e' && cmd[3] == 'a' && cmd[4] == 't' && cmd[5] == 'e' && cmd[6] == ' ') {
        // Extract filename from command
        const char* filename = &cmd[7];
        if (*filename == '\0') {
            print("Usage: create filename\n");
        } else {
            fs_create_file(filename);
        }
    } else if (cmd[0] == 'w' && cmd[1] == 'r' && cmd[2] == 'i' && cmd[3] == 't' && cmd[4] == 'e' && cmd[5] == ' ') {
        // Parse write command: write filename text
        const char* args = &cmd[6];
        
        // Skip spaces
        while (*args == ' ') args++;
        
        // Find filename
        const char* filename = args;
        int filename_len = 0;
        while (args[filename_len] && args[filename_len] != ' ') {
            filename_len++;
        }
        
        if (filename_len == 0) {
            print("Usage: write filename text\n");
            return;
        }
        
        // Extract filename
        char fname[MAX_FILENAME_LENGTH];
        int i;
        for (i = 0; i < filename_len && i < MAX_FILENAME_LENGTH - 1; i++) {
            fname[i] = filename[i];
        }
        fname[i] = '\0';
        
        // Find start of text
        const char* text = filename + filename_len;
        while (*text == ' ') text++;
        
        if (*text == '\0') {
            print("Usage: write filename text\n");
            return;
        }
        
        // Calculate text length
        int text_len = 0;
        while (text[text_len]) text_len++;
        
        // Write to file
        fs_write_file(fname, (const unsigned char*)text, text_len);
        
    } else if (cmd[0] == 'r' && cmd[1] == 'e' && cmd[2] == 'a' && cmd[3] == 'd' && cmd[4] == ' ') {
        // Extract filename from command
        const char* filename = &cmd[5];
        if (*filename == '\0') {
            print("Usage: read filename\n");
            return;
        }
        
        // Read file
        unsigned char buffer[FILE_SIZE + 1];  // +1 for null terminator
        int bytes_read = fs_read_file(filename, buffer, FILE_SIZE);
        
        if (bytes_read > 0) {
            print("File contents:\n");
            // Print as text (assuming text file)
            for (int i = 0; i < bytes_read; i++) {
                putchar(buffer[i]);
            }
            print("\n");
        }
    } else if (cmd[0] == 'd' && cmd[1] == 'e' && cmd[2] == 'l' && cmd[3] == 'e' && cmd[4] == 't' && cmd[5] == 'e' && cmd[6] == ' ') {
        // Extract filename from command
        const char* filename = &cmd[7];
        if (*filename == '\0') {
            print("Usage: delete filename\n");
        } else {
            fs_delete_file(filename);
        }
    } else {
        print("Unknown command: ");
        print(cmd);
        print("\nType 'help' for available commands.\n");
    }
}

// Simple shell
void run_shell() {
    print("\nType 'help' for available commands.\n\n");
    
    while (1) {
        print(">");
        
        // Save prompt position
        prompt_x = cursor_x;
        prompt_y = cursor_y;
        
        cmd_index = 0;
        
        // Read command
        while (1) {
            // Wait for keyboard input
            while (!keyboard_has_char()) {
                asm volatile("hlt");
            }
            
            char c = keyboard_getchar();
            
            if (c == '\n') {
                command_buffer[cmd_index] = '\0';
                putchar('\n');  // Move to next line before processing command
                process_command(command_buffer);
                break;
            } else if (c == '\b' && cmd_index > 0) {
                // Only process backspace if there are characters to delete
                // and not at the prompt position
                if (cursor_y > prompt_y || (cursor_y == prompt_y && cursor_x > prompt_x)) {
                    cmd_index--;
                    putchar('\b');
                }
            } else if (c >= 32 && cmd_index < CMD_BUFFER_SIZE - 1) {
                command_buffer[cmd_index++] = c;
                putchar(c);
            }
        }
    }
}

// Kernel entry point
void kernel_main() {
    clear_screen();
    
    enable_cursor();
    
    print("Kernel loaded successfully!\n");
    print("\n");
    
    print("Initializing IDT...\n");
    idt_init();
    
    print("Initializing keyboard...\n");
    keyboard_init();
    
    print("Initializing memory...\n");
    memory_init();
    print("Memory: 1MB at 0x200000\n");
    
    print("Initializing file system...\n");
    fs_init();
    
    print("Initializing process manager...\n");
    init_processes();
    
    print("Enabling interrupts...\n");
    asm volatile("sti");
    
    run_shell();
    
    while (1) {
        asm volatile("hlt");
    }
} 
