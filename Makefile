# Makefile 

# Tools
AS = nasm
CC = gcc
LD = ld
QEMU = qemu-system-i386

# Flags
ASFLAGS = -f elf32
CFLAGS = -m32 -ffreestanding -fno-pie -fno-pic -fno-stack-protector -nostdlib -nostdinc -Wall -Wextra -O0 -g
LDFLAGS = -m elf_i386 -T link.ld --print-map

# Targets
all: os.img

# Build bootloader
boot.bin: boot.asm
	$(AS) -f bin boot.asm -o boot.bin

# Build kernel entry
kernel_entry.o: kernel_entry.asm
	$(AS) $(ASFLAGS) kernel_entry.asm -o kernel_entry.o

# Build interrupt handlers
interrupt.o: interrupt.asm
	$(AS) $(ASFLAGS) interrupt.asm -o interrupt.o

# Build IDT
idt.o: idt.c idt.h
	$(CC) $(CFLAGS) -c idt.c -o idt.o

# Build keyboard driver
keyboard.o: keyboard.c keyboard.h
	$(CC) $(CFLAGS) -c keyboard.c -o keyboard.o

# Build memory manager
memory.o: memory.c memory.h
	$(CC) $(CFLAGS) -c memory.c -o memory.o

# Build file system
fs.o: fs.c fs.h
	$(CC) $(CFLAGS) -c fs.c -o fs.o

# Build kernel
kernel.o: kernel.c idt.h keyboard.h memory.h fs.h
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

# Link kernel 
kernel.elf: kernel_entry.o kernel.o idt.o interrupt.o keyboard.o memory.o fs.o
	$(LD) $(LDFLAGS) kernel_entry.o kernel.o idt.o interrupt.o keyboard.o memory.o fs.o -o kernel.elf > kernel.map

# Extract binary from ELF
kernel.bin: kernel.elf
	objcopy -O binary -j .text -j .rodata -j .data -j .bss kernel.elf kernel.bin

# Create OS image (bootloader + kernel)
os.img: boot.bin kernel.bin
	dd if=/dev/zero of=os.img bs=512 count=2880
	dd if=boot.bin of=os.img conv=notrunc
	dd if=kernel.bin of=os.img bs=512 seek=1 conv=notrunc

run: os.img
	$(QEMU) -fda os.img -display sdl -m 32M

debug: os.img
	$(QEMU) -drive format=raw,file=os.img -s -S -m 32M &
	gdb -ex "target remote localhost:1234" -ex "break *0x7c00"

clean:
	rm -f *.bin *.o *.img *.elf *.map

# Check symbols
symbols: kernel.elf
	nm kernel.elf | grep -E "(print|putchar|print_dec)" | sort
