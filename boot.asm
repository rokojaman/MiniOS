; boot.asm
[BITS 16]           ; Start in 16-bit Real Mode
[ORG 0x7C00]        ; BIOS loads boot sector to 0x7C00

start:
    ; Set up segments
    xor ax, ax      ; Zero out AX
    mov ds, ax      ; Set data segment to 0
    mov es, ax      ; Set extra segment to 0
    mov ss, ax      ; Set stack segment to 0
    mov sp, 0x7C00  ; Set stack pointer just below bootloader

    ; Clear screen
    mov ah, 0x00    ; Video mode function
    mov al, 0x03    ; 80x25 color text mode
    int 0x10        ; BIOS video interrupt

    ; Print message before loading kernel
    mov si, real_mode_msg
    call print_string_16

    ; Load kernel from disk (must be done in real mode)
    call load_kernel_16

    ; Print success message
    mov si, kernel_loaded_msg
    call print_string_16

    ; Load GDT
    lgdt [gdt_descriptor]

    ; Switch to protected mode
    mov eax, cr0
    or eax, 1       ; Set PE (Protection Enable) bit
    mov cr0, eax

    ; Far jump to flush CPU pipeline and enter 32-bit code
    jmp CODE_SEG:init_32bit

; 16-bit functions
print_string_16:
    pusha
.loop:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    mov bh, 0x00
    mov bl, 0x07
    int 0x10
    jmp .loop
.done:
    popa
    ret

; Load kernel from disk using BIOS INT 13h
load_kernel_16:
    pusha
    
    ; Reset disk system
    xor ax, ax
    mov dl, 0x00        ; Floppy disk
    int 0x13
    jc .disk_error
    
    ; Read kernel from disk
    ; BIOS read sectors function
    mov ah, 0x02        ; Read sectors function
    mov al, 40          ; Number of sectors to read (10KB)
    mov ch, 0           ; Cylinder 0
    mov cl, 2           ; Start from sector 2 (sector 1 is bootloader)
    mov dh, 0           ; Head 0
    mov dl, 0x00        ; First floppy disk
    mov bx, 0x1000      ; Load kernel to 0x1000:0x0000 = 0x10000
    mov es, bx
    xor bx, bx          ; Offset 0
    
    int 0x13            ; BIOS disk interrupt
    jc .disk_error      ; Jump if carry flag (error)
    
    cmp al, 40          ; Check if all sectors were read
    jne .disk_error
    
    ; Restore ES
    xor ax, ax
    mov es, ax
    
    popa
    ret

.disk_error:
    ; Show error code
    mov si, disk_error_msg
    call print_string_16
    
    ; Print error code in AH
    mov al, ah
    shr al, 4
    call print_hex_digit
    mov al, ah
    and al, 0x0F
    call print_hex_digit
    
    jmp $

; Print a single hex digit
print_hex_digit:
    and al, 0x0F
    add al, '0'
    cmp al, '9'
    jle .print
    add al, 7
.print:
    mov ah, 0x0E
    int 0x10
    ret

; GDT (Global Descriptor Table)
gdt_start:
    ; Null descriptor (required)
    dd 0x0
    dd 0x0

gdt_code:
    ; Code segment descriptor
    dw 0xFFFF       ; Limit (0-15)
    dw 0x0          ; Base (0-15)
    db 0x0          ; Base (16-23)
    db 10011010b    ; Access byte: present, ring 0, code segment, executable, readable
    db 11001111b    ; Flags (4 bits) + Limit (16-19): 4KB pages, 32-bit mode
    db 0x0          ; Base (24-31)

gdt_data:
    ; Data segment descriptor
    dw 0xFFFF       ; Limit (0-15)
    dw 0x0          ; Base (0-15)
    db 0x0          ; Base (16-23)
    db 10010010b    ; Access byte: present, ring 0, data segment, writable
    db 11001111b    ; Flags (4 bits) + Limit (16-19): 4KB pages, 32-bit mode
    db 0x0          ; Base (24-31)

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; Size of GDT
    dd gdt_start                ; Start address of GDT

; Segment selectors
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; 32-bit code section
[BITS 32]
init_32bit:
    ; Set up segments for 32-bit mode
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000    ; Set stack pointer to a safe location

    ; Print message in protected mode
    mov esi, protected_mode_msg
    call print_string_32

    ; Jump to kernel at 0x10000 (where we loaded it)
    jmp 0x10000

; 32-bit print function (prints at current position)
print_string_32:
    pusha
    mov edi, 0xB8000 + (80 * 2 * 2)   ; Start at line 3 (after real mode messages)
.loop:
    lodsb               ; Load byte from [ESI] into AL
    or al, al           ; Check for null terminator
    jz .done
    mov ah, 0x07        ; White on black attribute
    stosw               ; Store character + attribute to [EDI]
    jmp .loop
.done:
    popa
    ret

; Data
real_mode_msg db 'Starting in Real Mode...', 0x0D, 0x0A, 0
kernel_loaded_msg db 'Kernel loaded from disk!', 0x0D, 0x0A, 0
disk_error_msg db 'Disk read error! Code: ', 0
protected_mode_msg db 'Successfully entered 32-bit Protected Mode!', 0

; Pad boot sector to 510 bytes and add boot signature
times 510-($-$$) db 0
dw 0xAA55
