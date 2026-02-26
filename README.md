# SimpleOS

A minimal 32-bit x86 operating system built from scratch for educational purposes.  
Demonstrates boot process, protected mode transition, interrupt handling, memory allocation, scheduling, file system basics, and kernel design fundamentals.  
Deployed using [v86](https://copy.sh/v86/): [Link](http://jaman.dev)  

## Overview
- Custom bootloader (16-bit real mode → 32-bit protected mode)  
- C kernel with low-level Assembly integration  
- Runs in QEMU  

## Core System
- VGA text mode driver with cursor support  
- Full Interrupt Descriptor Table (IDT)  
- Hardware interrupt handling  
- PS/2 keyboard driver with shift/caps lock support  

## Memory Management
- Simple bump allocator (1MB heap at 0x200000)  
- Memory usage statistics  
- Fragmentation-free linear allocation  

## Process Management
- Process table (up to 8 processes)  
- States: READY, RUNNING, BLOCKED, ZOMBIE  
- Round-robin scheduler (~1s time slice)  
- Basic Process Control Blocks (PID, state, name)  

## File System
- In-memory file system (16 files × 512 bytes)  
- Create, read, write, delete operations  
- Fixed-size storage (8KB total)  

## Command Shell
- Interactive CLI  
- Built-in commands for memory, processes, and file management  

## Technical Highlights
- Manual memory layout configuration  
- Custom linker script and low-level build system  
- Direct hardware interaction (VGA, keyboard, interrupts)  

## Limitations
- No persistence (RAM-only file system)  
- No paging or user/kernel separation  
- No true multitasking  
