/* link.ld

ENTRY(_start)

SECTIONS
{
    . = 0x10000;  /* Kernel will be loaded at 0x10000 */
    
    .text : AT(0x10000)
    {
        *(.text)
    }
    
    .rodata : ALIGN(4K)
    {
        *(.rodata)
    }
    
    .data : ALIGN(4K)
    {
        *(.data)
    }
    
    .bss : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
    }
    
    /* Export kernel_end symbol - make it global */
    . = ALIGN(4K);
    kernel_end = .;
    PROVIDE(kernel_end = .);
}
