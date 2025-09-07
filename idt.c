// idt.c 

#include "idt.h"

// IDT entries
struct idt_entry idt[256];
struct idt_ptr idtp;

// External assembly interrupt handlers 
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

// IRQ handlers
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

// Print function from kernel.c
extern void print(const char* str);
extern void putchar(char c);

// Helper function to print hex numbers (static to avoid multiple definition)
static void print_hex(unsigned int n) {
    char hex_chars[] = "0123456789ABCDEF";
    print("0x");
    for (int i = 7; i >= 0; i--) {
        putchar(hex_chars[(n >> (i * 4)) & 0xF]);
    }
}

// Set an IDT gate
void idt_set_gate(unsigned char num, unsigned int base, unsigned short sel, unsigned char flags) {
    idt[num].base_lo = (base & 0xFFFF);
    idt[num].base_hi = (base >> 16) & 0xFFFF;
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

// Remap the PIC (Programmable Interrupt Controller)
void remap_pic() {
    unsigned char a1, a2;
    
    // Save masks
    a1 = inb(0x21);
    a2 = inb(0xA1);
    
    // Start initialization sequence
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    
    // Set vector offsets
    outb(0x21, 0x20);  // Master PIC vector offset (0x20 = 32)
    outb(0xA1, 0x28);  // Slave PIC vector offset (0x28 = 40)
    
    // Tell Master PIC there's a slave at IRQ2
    outb(0x21, 0x04);
    // Tell Slave PIC its cascade identity
    outb(0xA1, 0x02);
    
    // Set 8086 mode
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    
    // Restore saved masks
    outb(0x21, a1);
    outb(0xA1, a2);
}

// Initialize the IDT
void idt_init() {
    // Set IDT pointer
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (unsigned int)&idt;
    
    // Clear IDT
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }
    
    // Set up CPU exception handlers (ISRs 0-31)
    idt_set_gate(0, (unsigned int)isr0, 0x08, 0x8E);
    idt_set_gate(1, (unsigned int)isr1, 0x08, 0x8E);
    idt_set_gate(2, (unsigned int)isr2, 0x08, 0x8E);
    idt_set_gate(3, (unsigned int)isr3, 0x08, 0x8E);
    idt_set_gate(4, (unsigned int)isr4, 0x08, 0x8E);
    idt_set_gate(5, (unsigned int)isr5, 0x08, 0x8E);
    idt_set_gate(6, (unsigned int)isr6, 0x08, 0x8E);
    idt_set_gate(7, (unsigned int)isr7, 0x08, 0x8E);
    idt_set_gate(8, (unsigned int)isr8, 0x08, 0x8E);
    idt_set_gate(9, (unsigned int)isr9, 0x08, 0x8E);
    idt_set_gate(10, (unsigned int)isr10, 0x08, 0x8E);
    idt_set_gate(11, (unsigned int)isr11, 0x08, 0x8E);
    idt_set_gate(12, (unsigned int)isr12, 0x08, 0x8E);
    idt_set_gate(13, (unsigned int)isr13, 0x08, 0x8E);
    idt_set_gate(14, (unsigned int)isr14, 0x08, 0x8E);
    idt_set_gate(15, (unsigned int)isr15, 0x08, 0x8E);
    idt_set_gate(16, (unsigned int)isr16, 0x08, 0x8E);
    idt_set_gate(17, (unsigned int)isr17, 0x08, 0x8E);
    idt_set_gate(18, (unsigned int)isr18, 0x08, 0x8E);
    idt_set_gate(19, (unsigned int)isr19, 0x08, 0x8E);
    idt_set_gate(20, (unsigned int)isr20, 0x08, 0x8E);
    idt_set_gate(21, (unsigned int)isr21, 0x08, 0x8E);
    idt_set_gate(22, (unsigned int)isr22, 0x08, 0x8E);
    idt_set_gate(23, (unsigned int)isr23, 0x08, 0x8E);
    idt_set_gate(24, (unsigned int)isr24, 0x08, 0x8E);
    idt_set_gate(25, (unsigned int)isr25, 0x08, 0x8E);
    idt_set_gate(26, (unsigned int)isr26, 0x08, 0x8E);
    idt_set_gate(27, (unsigned int)isr27, 0x08, 0x8E);
    idt_set_gate(28, (unsigned int)isr28, 0x08, 0x8E);
    idt_set_gate(29, (unsigned int)isr29, 0x08, 0x8E);
    idt_set_gate(30, (unsigned int)isr30, 0x08, 0x8E);
    idt_set_gate(31, (unsigned int)isr31, 0x08, 0x8E);
    
    // Remap PIC
    remap_pic();
    
    // Set up hardware interrupt handlers (IRQs 0-15)
    idt_set_gate(32, (unsigned int)irq0, 0x08, 0x8E);
    idt_set_gate(33, (unsigned int)irq1, 0x08, 0x8E);
    idt_set_gate(34, (unsigned int)irq2, 0x08, 0x8E);
    idt_set_gate(35, (unsigned int)irq3, 0x08, 0x8E);
    idt_set_gate(36, (unsigned int)irq4, 0x08, 0x8E);
    idt_set_gate(37, (unsigned int)irq5, 0x08, 0x8E);
    idt_set_gate(38, (unsigned int)irq6, 0x08, 0x8E);
    idt_set_gate(39, (unsigned int)irq7, 0x08, 0x8E);
    idt_set_gate(40, (unsigned int)irq8, 0x08, 0x8E);
    idt_set_gate(41, (unsigned int)irq9, 0x08, 0x8E);
    idt_set_gate(42, (unsigned int)irq10, 0x08, 0x8E);
    idt_set_gate(43, (unsigned int)irq11, 0x08, 0x8E);
    idt_set_gate(44, (unsigned int)irq12, 0x08, 0x8E);
    idt_set_gate(45, (unsigned int)irq13, 0x08, 0x8E);
    idt_set_gate(46, (unsigned int)irq14, 0x08, 0x8E);
    idt_set_gate(47, (unsigned int)irq15, 0x08, 0x8E);
    
    // Load the IDT
    idt_load((unsigned int)&idtp);
    
    print("IDT initialized\n");
}

// Exception messages
const char *exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

// ISR handler
void isr_handler(struct registers regs) {
    print("Exception: ");
    print(exception_messages[regs.int_no]);
    print(" (");
    print_hex(regs.int_no);
    print(")\n");
    
    if (regs.err_code) {
        print("Error code: ");
        print_hex(regs.err_code);
        print("\n");
    }
    
    // Halt on exception
    while (1) {
        asm volatile("hlt");
    }
}

// External keyboard handler
extern void keyboard_handler();

// External timer handler
extern void timer_handler();

// IRQ handler
void irq_handler(struct registers regs) {
    // Send EOI (End of Interrupt) signal to PICs
    if (regs.int_no >= 40) {
        // Send to slave PIC
        outb(0xA0, 0x20);
    }
    // Send to master PIC
    outb(0x20, 0x20);
    
    // Handle specific IRQs
    switch(regs.int_no) {
        case 32:  // Timer (IRQ0)
            timer_handler();
            break;
        case 33:  // Keyboard (IRQ1)
            keyboard_handler();
            break;
        default:
            // Ignore other IRQs for now
            break;
    }
}
