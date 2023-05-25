#ifndef FILE_SYSTEMS_H
#define FILE_SYSTEMS_H

#include "lib.h"
#include "types.h"

#define DENTRY_RESERVED 24
#define BOOT_RESERVED   52
#define NUM_OF_DENTRY   63
#define INODE_DATA_NUM  1023 
#define DENTRY_NUM 64 
#define INODE_BLOCK_SIZE 4096 
#define FILE_BOUND 32
#define INODE_FIND 1   
#define BEGIN_FILE_DESC 2 

// Initializing dentry struct
typedef struct {
    uint8_t filename[FILE_BOUND];
    uint32_t filetype;
    uint32_t inode_num;
    uint8_t reserved[DENTRY_RESERVED];
} dentry_t;

// Initializing bloot block struct
typedef struct {
    uint32_t dentries_number;
    uint32_t inodes_number;
    uint32_t data_blocks_number;
    uint8_t reserved[BOOT_RESERVED];
    dentry_t dentry[NUM_OF_DENTRY];
} boot_block_t;

// Initializing inode struct
typedef struct {
    int32_t length;
    int32_t data_blocks_num[INODE_DATA_NUM];
} inode_t;

// Initializing required blocks, pointers, entries and inodes
boot_block_t* ptr_boot_block;
uint8_t* ptr_data;
inode_t* ptr_inode;
uint32_t inodes;
uint32_t dentries;
dentry_t* ptr_dentry;
uint32_t data_blocks_total;
uint32_t file_pos;

// Initializing required functions we use
void file_system_init(uint32_t files_input);
int32_t read_dentry_by_name (const uint8_t* file_name, dentry_t* dentry);
int32_t read_dentry_by_index (uint8_t index, dentry_t* dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
int32_t file_open(const uint8_t* file_name);
int32_t file_close(int32_t fd);
int32_t file_read(int32_t fd, void* buf, int32_t nbytes);
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t dir_open(const uint8_t* file_name);
int32_t dir_close(int32_t fd);
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes);
int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes);

#endif 
