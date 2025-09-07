// keyboard.c 

#include "keyboard.h"
#include "idt.h"

// External functions from kernel
extern void putchar(char c);
extern void print(const char* str);

// Keyboard data port
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

// Special key states
static unsigned char shift_pressed = 0;
static unsigned char ctrl_pressed = 0;
static unsigned char alt_pressed = 0;
static unsigned char caps_lock = 0;

// Keyboard buffer
#define KEYBOARD_BUFFER_SIZE 256
static char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
static int buffer_start = 0;
static int buffer_end = 0;

// US QWERTZ keyboard scancode to ASCII lookup tables
// Normal keys (without shift)
static const unsigned char scancode_to_ascii[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',     /* 0-9 */
    '9', '0', '\'', '=', '\b',                           /* 10-14: Backspace */
    '\t',                                                /* 15: Tab */
    'q', 'w', 'e', 'r', 't', 'z', 'u', 'i', 'o', 'p',  /* 16-25 */    // i replaced z and y keys to match my keyboard
    'š', 'đ', '\n',                                      /* 26-28: Enter */
    0,                                                   /* 29: Left Ctrl */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'č',  /* 30-39 */
    'ć', 'ž',                                           /* 40-41 */
    0,                                                   /* 42: Left Shift */
    '\\', 'y', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', /* 43-52 */
    '-',                                                 /* 53 */
    0,                                                   /* 54: Right Shift */
    '*',                                                 /* 55: Keypad * */
    0,                                                   /* 56: Left Alt */
    ' ',                                                 /* 57: Space */
    0,                                                   /* 58: Caps Lock */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                      /* 59-68: F1-F10 */
    0,                                                   /* 69: Num Lock */
    0,                                                   /* 70: Scroll Lock */
    0, 0, 0,                                            /* 71-73: Keypad 7,8,9 */
    '-',                                                 /* 74: Keypad - */
    0, 0, 0,                                            /* 75-77: Keypad 4,5,6 */
    '+',                                                 /* 78: Keypad + */
    0, 0, 0,                                            /* 79-81: Keypad 1,2,3 */
    0,                                                   /* 82: Keypad 0 */
    0,                                                   /* 83: Keypad . */
    0, 0, 0,                                            /* 84-86 */
    0,                                                   /* 87: F11 */
    0,                                                   /* 88: F12 */
    0,                                                   /* 89+ */
};

// Shifted keys
static const unsigned char scancode_to_ascii_shift[128] = {
    0,  27, '!', '"', '#', '$', '%', '&', '/', '(',     /* 0-9 */
    ')', '=', '?', '*', '\b',                           /* 10-14: Backspace */
    '\t',                                                /* 15: Tab */
    'Q', 'W', 'E', 'R', 'T', 'Z', 'U', 'I', 'O', 'P',  /* 16-25 */
    'Š', 'Đ', '\n',                                      /* 26-28: Enter */
    0,                                                   /* 29: Left Ctrl */
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'Č',  /* 30-39 */
    'Ć', 'Ž',                                            /* 40-41 */
    0,                                                   /* 42: Left Shift */
    '>', 'Y', 'X', 'C', 'V', 'B', 'N', 'M', ';', ':',  /* 43-52 */
    '_',                                                 /* 53 */
    0,                                                   /* 54: Right Shift */
    '*',                                                 /* 55: Keypad * */
    0,                                                   /* 56: Left Alt */
    ' ',                                                 /* 57: Space */
    0,                                                   /* 58: Caps Lock */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                      /* 59-68: F1-F10 */
    0,                                                   /* 69: Num Lock */
    0,                                                   /* 70: Scroll Lock */
    '7', '8', '9',                                      /* 71-73: Keypad 7,8,9 */
    '-',                                                 /* 74: Keypad - */
    '4', '5', '6',                                      /* 75-77: Keypad 4,5,6 */
    '+',                                                 /* 78: Keypad + */
    '1', '2', '3',                                      /* 79-81: Keypad 1,2,3 */
    '0',                                                 /* 82: Keypad 0 */
    '.',                                                 /* 83: Keypad . */
    0, 0, 0,                                            /* 84-86 */
    0,                                                   /* 87: F11 */
    0,                                                   /* 88: F12 */
    0,                                                   /* 89+ */
};

// Add character to keyboard buffer
static void add_to_buffer(char c) {
    int next_end = (buffer_end + 1) % KEYBOARD_BUFFER_SIZE;
    if (next_end != buffer_start) {  // Buffer not full
        keyboard_buffer[buffer_end] = c;
        buffer_end = next_end;
    }
}

// Read character from keyboard buffer
char keyboard_getchar() {
    if (buffer_start == buffer_end) {
        return 0;  // Buffer empty
    }
    
    char c = keyboard_buffer[buffer_start];
    buffer_start = (buffer_start + 1) % KEYBOARD_BUFFER_SIZE;
    return c;
}

// Check if keyboard buffer has data
int keyboard_has_char() {
    return buffer_start != buffer_end;
}

// Process scancode and convert to ASCII
static void process_scancode(unsigned char scancode) {
    // Check if it's a key release (high bit set)
    if (scancode & 0x80) {
        scancode &= 0x7F;  // Clear high bit
        
        // Handle special key releases
        switch (scancode) {
            case 0x2A:  // Left Shift
            case 0x36:  // Right Shift
                shift_pressed = 0;
                break;
            case 0x1D:  // Ctrl
                ctrl_pressed = 0;
                break;
            case 0x38:  // Alt
                alt_pressed = 0;
                break;
        }
        return;
    }
    
    // Handle special key presses
    switch (scancode) {
        case 0x2A:  // Left Shift
        case 0x36:  // Right Shift
            shift_pressed = 1;
            return;
        case 0x1D:  // Ctrl
            ctrl_pressed = 1;
            return;
        case 0x38:  // Alt
            alt_pressed = 1;
            return;
        case 0x3A:  // Caps Lock
            caps_lock = !caps_lock;
            return;
    }
    
    // Convert scancode to ASCII
    char ascii = 0;
    if (shift_pressed) {
        ascii = scancode_to_ascii_shift[scancode];
    } else {
        ascii = scancode_to_ascii[scancode];
    }
    
    // Handle caps lock for letters
    if (caps_lock && ascii >= 'a' && ascii <= 'z' && !shift_pressed) {
        ascii -= 32;  // Convert to uppercase
    } else if (caps_lock && ascii >= 'A' && ascii <= 'Z' && shift_pressed) {
        ascii += 32;  // Convert to lowercase
    }
    
    // Handle Ctrl combinations
    if (ctrl_pressed && ascii >= 'a' && ascii <= 'z') {
        ascii -= 96;  // Ctrl+A = 1, Ctrl+B = 2, etc.
    }
    
    // Add to buffer (shell will handle echo)
    if (ascii) {
        add_to_buffer(ascii);
    }
}

// Keyboard interrupt handler (called from IRQ1)
void keyboard_handler() {
    // Read scancode from keyboard controller
    unsigned char scancode = inb(KEYBOARD_DATA_PORT);
    
    // Process the scancode
    process_scancode(scancode);
}

// Initialize keyboard driver
void keyboard_init() {
    // Clear keyboard buffer
    buffer_start = 0;
    buffer_end = 0;
    
    // Clear key states
    shift_pressed = 0;
    ctrl_pressed = 0;
    alt_pressed = 0;
    caps_lock = 0;
    
    // Flush keyboard buffer
    while (inb(KEYBOARD_STATUS_PORT) & 1) {
        inb(KEYBOARD_DATA_PORT);
    }
    
    print("Keyboard driver initialized\n");
}
