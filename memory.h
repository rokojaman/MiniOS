// memory.h

#ifndef MEMORY_H
#define MEMORY_H

// Functions
void memory_init();
void* malloc(unsigned int size);
void memory_stats();
void free_all();
void memory_free();
void memory_used();
void memory_register_fs(void* addr, unsigned int size);

#endif
