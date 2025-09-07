// idt.h

#ifndef IDT_H
#define IDT_H

// IDT entry structure
struct idt_entry {
    unsigned short base_lo;  // Lower 16 bits of handler address
    unsigned short sel;      // Kernel segment selector
    unsigned char always0;   // Always 0
    unsigned char flags;     // Flags
    unsigned short base_hi;  // Upper 16 bits of handler address
} __attribute__((packed));

// IDT pointer structure
struct idt_ptr {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

// Registers structure for interrupt handlers
struct registers {
    unsigned int ds;                                     // Data segment selector
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha
    unsigned int int_no, err_code;                      // Interrupt number and error code
    unsigned int eip, cs, eflags, useresp, ss;          // Pushed by processor
};

// Function declarations
void idt_init();
void idt_set_gate(unsigned char num, unsigned int base, unsigned short sel, unsigned char flags);

// Assembly functions
extern void idt_load(unsigned int);

// Port I/O functions
static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(unsigned short port, unsigned char val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Interrupt handlers
void isr_handler(struct registers regs);
void irq_handler(struct registers regs);

#endif
