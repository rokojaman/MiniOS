// keyboard.h

#ifndef KEYBOARD_H
#define KEYBOARD_H

// Initialize keyboard driver
void keyboard_init();

// Keyboard interrupt handler
void keyboard_handler();

// Get a character from keyboard buffer (returns 0 if buffer empty)
char keyboard_getchar();

// Check if keyboard buffer has characters
int keyboard_has_char();

#endif
