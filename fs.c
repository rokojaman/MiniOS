// fs.c

#include "fs.h"
#include "memory.h"

// External functions from kernel
extern void print(const char* str);
extern void print_dec(unsigned int n);
extern void putchar(char c);

// External function from memory manager
extern void memory_register_fs(void* addr, unsigned int size);

// Global file system instance
static struct fs filesystem;

// Initialize the file system
void fs_init(void) {
    
    // Allocate memory for data area
    // Total size: MAX_FILES * FILE_SIZE
    unsigned int total_size = MAX_FILES * FILE_SIZE;
    filesystem.data_area = (unsigned char*)malloc(total_size);
    
    if (!filesystem.data_area) {
        print("Failed to allocate memory for file system!\n");
        return;
    }
    
    // Register this allocation with memory manager so it won't be freed
    memory_register_fs(filesystem.data_area, total_size);
    
    filesystem.data_size = total_size;
    
    // Initialize all file entries as free
    for (int i = 0; i < MAX_FILES; i++) {
        filesystem.files[i].flags = FILE_FREE;
        filesystem.files[i].size = 0;
        filesystem.files[i].data_offset = i * FILE_SIZE;  // Each file gets FILE_SIZE bytes
        
        // Clear filename
        for (int j = 0; j < MAX_FILENAME_LENGTH; j++) {
            filesystem.files[i].name[j] = '\0';
        }
    }
    
    // Clear data area
    for (unsigned int i = 0; i < total_size; i++) {
        filesystem.data_area[i] = 0;
    }
    
    filesystem.initialized = 1;
    
    print("File system initialized: ");
    print_dec(MAX_FILES);
    print(" files, ");
    print_dec(FILE_SIZE);
    print(" bytes each (");
    print_dec(total_size);
    print(" bytes total)\n");
}

// Helper function to compare strings
static int str_compare(const char* s1, const char* s2) {
    while (*s1 && *s2) {
        if (*s1 != *s2) return 0;
        s1++;
        s2++;
    }
    return (*s1 == *s2);
}

// Helper function to copy strings
static void str_copy(char* dest, const char* src, int max_len) {
    int i;
    for (i = 0; i < max_len - 1 && src[i]; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0';
}

// Create a new file
int fs_create_file(const char* name) {
    if (!filesystem.initialized) {
        print("File system not initialized!\n");
        return -1;
    }
    
    // Check if name is too long
    int name_len = 0;
    while (name[name_len] && name_len < MAX_FILENAME_LENGTH) {
        name_len++;
    }
    if (name_len >= MAX_FILENAME_LENGTH) {
        print("Filename too long!\n");
        return -1;
    }
    
    // Check if file already exists
    for (int i = 0; i < MAX_FILES; i++) {
        if (filesystem.files[i].flags & FILE_USED) {
            if (str_compare(filesystem.files[i].name, name)) {
                print("File already exists!\n");
                return -1;
            }
        }
    }
    
    // Find a free slot
    for (int i = 0; i < MAX_FILES; i++) {
        if (!(filesystem.files[i].flags & FILE_USED)) {
            // Found free slot
            filesystem.files[i].flags = FILE_USED;
            filesystem.files[i].size = 0;  // Start with empty file
            str_copy(filesystem.files[i].name, name, MAX_FILENAME_LENGTH);
            
            // Clear the file's data area
            unsigned char* data = filesystem.data_area + filesystem.files[i].data_offset;
            for (int j = 0; j < FILE_SIZE; j++) {
                data[j] = 0;
            }
            
            print("File created: ");
            print(name);
            print("\n");
            return i;  // Return file index
        }
    }
    
    print("No free file slots!\n");
    return -1;
}

// Delete a file
int fs_delete_file(const char* name) {
    if (!filesystem.initialized) {
        print("File system not initialized!\n");
        return -1;
    }
    
    // Find the file
    int file_index = -1;
    for (int i = 0; i < MAX_FILES; i++) {
        if (filesystem.files[i].flags & FILE_USED) {
            if (str_compare(filesystem.files[i].name, name)) {
                file_index = i;
                break;
            }
        }
    }
    
    if (file_index == -1) {
        print("File not found!\n");
        return -1;
    }
    
    // Mark file as free
    filesystem.files[file_index].flags = FILE_FREE;
    filesystem.files[file_index].size = 0;
    
    // Clear filename
    for (int i = 0; i < MAX_FILENAME_LENGTH; i++) {
        filesystem.files[file_index].name[i] = '\0';
    }
    
    // Clear file data (optional, but good for security)
    unsigned char* file_data = filesystem.data_area + filesystem.files[file_index].data_offset;
    for (int i = 0; i < FILE_SIZE; i++) {
        file_data[i] = 0;
    }
    
    print("File deleted: ");
    print(name);
    print("\n");
    
    return 0;
}

// Read data from a file
int fs_read_file(const char* name, unsigned char* buffer, unsigned int size) {
    if (!filesystem.initialized) {
        print("File system not initialized!\n");
        return -1;
    }
    
    // Find the file
    int file_index = -1;
    for (int i = 0; i < MAX_FILES; i++) {
        if (filesystem.files[i].flags & FILE_USED) {
            if (str_compare(filesystem.files[i].name, name)) {
                file_index = i;
                break;
            }
        }
    }
    
    if (file_index == -1) {
        print("File not found!\n");
        return -1;
    }
    
    // Determine how much to read
    unsigned int read_size = filesystem.files[file_index].size;
    if (read_size > size) {
        read_size = size;  // Don't overflow buffer
    }
    
    // Read data from file
    unsigned char* file_data = filesystem.data_area + filesystem.files[file_index].data_offset;
    for (unsigned int i = 0; i < read_size; i++) {
        buffer[i] = file_data[i];
    }
    
    return read_size;
}

// Write data to a file
int fs_write_file(const char* name, const unsigned char* data, unsigned int size) {
    if (!filesystem.initialized) {
        print("File system not initialized!\n");
        return -1;
    }
    
    // Find the file
    int file_index = -1;
    for (int i = 0; i < MAX_FILES; i++) {
        if (filesystem.files[i].flags & FILE_USED) {
            if (str_compare(filesystem.files[i].name, name)) {
                file_index = i;
                break;
            }
        }
    }
    
    if (file_index == -1) {
        print("File not found!\n");
        return -1;
    }
    
    // Check size limit
    if (size > FILE_SIZE) {
        print("Data too large! Max size: ");
        print_dec(FILE_SIZE);
        print(" bytes\n");
        return -1;
    }
    
    // Write data to file
    unsigned char* file_data = filesystem.data_area + filesystem.files[file_index].data_offset;
    for (unsigned int i = 0; i < size; i++) {
        file_data[i] = data[i];
    }
    
    // Update file size
    filesystem.files[file_index].size = size;
    
    print("Wrote ");
    print_dec(size);
    print(" bytes to ");
    print(name);
    print("\n");
    
    return size;
}

void fs_list_files(void) {
    if (!filesystem.initialized) {
        print("File system not initialized!\n");
        return;
    }
    
    int file_count = 0;
    
    print("Files in system:\n");
    print("Name            Size\n");
    print("----            ----\n");
    
    for (int i = 0; i < MAX_FILES; i++) {
        if (filesystem.files[i].flags & FILE_USED) {
            // Print filename (pad to 14 chars)
            print(filesystem.files[i].name);
            int name_len = 0;
            while (filesystem.files[i].name[name_len]) name_len++;
            for (int j = name_len; j < 14; j++) {
                putchar(' ');
            }
            
            // Print size
            print("  ");
            print_dec(filesystem.files[i].size);
            print(" bytes\n");
            
            file_count++;
        }
    }
    
    if (file_count == 0) {
        print("(No files)\n");
    } else {
        print("\nTotal: ");
        print_dec(file_count);
        print(" file(s)\n");
    }
}
