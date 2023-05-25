#ifndef PAGING_H
#define PAGING_H

#include "types.h"

#define VIDEO_MEMS      0xB8000
#define DIFF_VIDEO      0xB7000
#define TABLES_NUM      1024
#define FOUR_KB         4096
#define UPPER_20        12
#define VIDEO_INDEX     34

// Initializing all required structs
struct __attribute__ ((packed)) page_directory_entry_4kB_t {
            uint32_t present       : 1;
            uint32_t read_or_write : 1 ;
            uint32_t user_or_supervisor : 1 ;
            uint32_t page_write_through  : 1;
            uint32_t page_cache_disabled  : 1;
            uint32_t accessed :1 ;
            uint32_t set_zero : 1 ;
            uint32_t page_size : 1  ;
            uint32_t global_bit  : 1;
            uint32_t avail    : 3   ;
            uint32_t base_address : 20 ;
}page_directory_entry_4kB_t ;


extern void loadPageDirectory(unsigned int*);
extern void enablePaging();
extern void page_init();
extern void terminal_memory(int index);

struct __attribute__ ((packed)) page_table_entry_t {
            uint32_t present       : 1;
            uint32_t read_or_write : 1 ;
            uint32_t user_or_supervisor : 1 ;
            uint32_t page_write_through  : 1;
            uint32_t page_cache_disabled  : 1;
            uint32_t accessed :1 ;
            uint32_t dirty : 1  ;
            uint32_t page_attribute_table : 1  ;
            uint32_t global_bit  : 1;
            uint32_t avail    : 3   ;
            uint32_t base_address : 20 ;
}page_table_entry_t;

struct  __attribute__ ((packed)) page_directory_entry_4mB_t {
            uint32_t present       : 1;
            uint32_t read_or_write : 1 ;
            uint32_t user_or_supervisor : 1 ;
            uint32_t page_write_through  : 1;
            uint32_t page_cache_disabled  : 1;
            uint32_t accessed :1 ;
            uint32_t dirty :1 ;
            uint32_t page_size : 1  ;
            uint32_t global_bit  : 1;
            uint32_t avail    : 3   ;
            uint32_t page_attribute_table : 1  ;
            uint32_t reserved : 9 ; 
            uint32_t base_address : 10 ;
} page_directory_entry_4mB_t;


typedef union page_dirs {
    struct page_directory_entry_4kB_t p4k ; 
    struct page_directory_entry_4mB_t p4m ;
}page_dir_entries;

// Initializing page table entries and the main page directory
page_dir_entries pages_directory[TABLES_NUM] __attribute__((aligned(FOUR_KB)));
struct page_table_entry_t video_map_dir[TABLES_NUM] __attribute__((aligned(FOUR_KB)));
struct page_table_entry_t page_4k_tables[TABLES_NUM]  __attribute__((aligned(FOUR_KB))); // Initializing the PTE

#endif
