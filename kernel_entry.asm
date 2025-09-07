; kernel_entry.asm
[BITS 32]
[EXTERN kernel_main]

section .text
global _start

_start:
    ; Set up the stack
    mov esp, 0x90000
    
    ; Call the C kernel main function
    call kernel_main
    
    ; If kernel_main returns (it shouldn't), halt
    jmp $
