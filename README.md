# SimpleOS

A minimal operating system built from scratch for educational purposes, demonstrating fundamental OS concepts in x86 architecture.

## Features

### Core System
- **32-bit Protected Mode**: Transitions from 16-bit real mode to 32-bit protected mode
- **Custom Bootloader**: Loads kernel from disk and sets up the environment
- **VGA Text Mode Display**: Direct screen output with cursor support
- **Interrupt Handling**: Complete IDT (Interrupt Descriptor Table) implementation
- **PS/2 Keyboard Driver**: Full keyboard input with shift/caps lock support

### Memory Management
- **Simple Allocator**: Bump allocator with 1MB heap at address 0x200000
- **Memory Statistics**: Track used and free memory
- **No Fragmentation**: Simple design prevents memory fragmentation

### Process Management
- **Process Table**: Support for up to 8 concurrent processes
- **Process States**: READY, RUNNING, BLOCKED, and ZOMBIE states
- **Round-Robin Scheduler**: Basic time-slice scheduling (every ~1 second)
- **Process Control Block (PCB)**: Tracks PID, state, and process name

### File System
- **In-Memory Storage**: 16 file slots, 512 bytes each
- **File Operations**: Create, read, write, and delete files
- **Simple Design**: No directories, fixed-size files
- **8KB Total Storage**: Pre-allocated at system initialization

### Command Shell
Interactive command-line interface with built-in commands:

| Command | Description |
|---------|-------------|
| `help` | Display available commands |
| `clear` | Clear the screen |
| `about` | Show system information |
| `echo [text]` | Echo text to screen |
| `mem` | Display memory statistics |
| `memtest` | Test memory allocation |
| `memfree` | Free all allocated memory |
| `ps` | List running processes |
| `run` | Create a test process |
| `ls` | List all files |
| `create [filename]` | Create a new file |
| `write [filename] [text]` | Write text to file |
| `read [filename]` | Display file contents |
| `delete [filename]` | Delete a file |

## Building and Running

### Prerequisites
- NASM assembler
- GCC (with 32-bit support)
- GNU LD linker
- QEMU x86 emulator
- Make build tool

### Build Instructions
```bash
# Clean previous build
make clean

# Build the OS image
make

# Run in QEMU
make run

# Debug with GDB (optional)
make debug
```

## Project Structure

```
.
├── boot.asm          # Bootloader (real mode → protected mode)
├── kernel_entry.asm  # Kernel entry point
├── kernel.c          # Main kernel and shell
├── interrupt.asm     # Low-level interrupt handlers
├── idt.c/h          # Interrupt Descriptor Table
├── keyboard.c/h      # PS/2 keyboard driver
├── simple_memory.c/h # Memory allocator
├── simple_fs.c/h     # File system implementation
├── link.ld          # Linker script
└── Makefile         # Build configuration
```

## Technical Details

### Memory Layout
- `0x7C00`: Bootloader load address
- `0x10000`: Kernel load address
- `0x90000`: Kernel stack
- `0x200000`: Heap start (1MB)
- `0xB8000`: VGA text buffer

### File System Specifications
- **Max Files**: 16
- **File Size**: 512 bytes (fixed)
- **Filename Length**: 12 characters max
- **Total Storage**: 8KB
- **File Operations**: Sequential read/write

### Limitations
- No persistent storage (RAM-based file system)
- Fixed-size files only
- No subdirectories
- No file permissions or attributes
- Basic process management (no actual multitasking)
- Limited to 1MB of manageable memory

## Educational Value

This OS demonstrates:
- x86 boot process and protected mode transition
- Hardware interrupt handling
- Memory management concepts
- Process scheduling basics
- Simple file system design
- Direct hardware interaction (keyboard, VGA)

## Future Enhancements

Possible extensions for learning:
- Paging and virtual memory
- Actual process context switching
- Dynamic memory allocation (malloc/free)
- Persistent storage (hard disk driver)
- Multi-level file system with directories
- User mode/kernel mode separation
- System calls
- Basic networking

## License

This is an educational project intended for learning operating system concepts.

## Acknowledgments

Built as a learning project to understand fundamental OS concepts including bootloaders, kernel development, interrupt handling, memory management, process scheduling, and file systems.
