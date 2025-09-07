// fs.h

#ifndef FS_H
#define FS_H

// File system constants
#define MAX_FILES 16           // Maximum number of files
#define MAX_FILENAME_LENGTH 12 // 8.3 format (8 chars + dot + 3 chars)
#define FILE_SIZE 512          // Fixed file size in bytes

// File flags
#define FILE_FREE 0x00
#define FILE_USED 0x01

// File entry structure (directory entry)
struct file_entry {
    char name[MAX_FILENAME_LENGTH];  // File name
    unsigned char flags;             // File flags (free/used)
    unsigned int size;               // File size (always FILE_SIZE for now)
    unsigned int data_offset;        // Offset to file data
};

// File system structure
struct fs {
    struct file_entry files[MAX_FILES];  // File directory
    unsigned char* data_area;            // Pointer to data storage area
    unsigned int data_size;              // Total size of data area
    int initialized;                     // Is file system initialized?
};

// File system functions
void fs_init(void);
int fs_create_file(const char* name);
int fs_delete_file(const char* name);
int fs_read_file(const char* name, unsigned char* buffer, unsigned int size);
int fs_write_file(const char* name, const unsigned char* data, unsigned int size);
void fs_list_files(void);

#endif
