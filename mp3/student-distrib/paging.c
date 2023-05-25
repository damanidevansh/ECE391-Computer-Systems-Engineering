#include "paging.h"

/*
 * page_init
 *  DESCRIPTION: Initializes paging and sets every page dentry and table's required bits to 1 or 0.
 *  INPUTS: None
 *  OUTPUTS: None
 *  RETURN VALUE: None
 *  SIDE EFFECTS: Makes paging work!
 */
void page_init() 
{
    int i;
    int index = VIDEO_MEMS;
    index /= FOUR_KB;
    
    // Working with the dentry struct
    for (i = 0; i < TABLES_NUM; i++) {
        if (i == 0) { // Setting up the boot block
            pages_directory[i].p4k.present = 1;
            pages_directory[i].p4k.read_or_write = 1;
            pages_directory[i].p4k.user_or_supervisor = 0; 
            pages_directory[i].p4k.page_write_through = 0;
            pages_directory[i].p4k.page_cache_disabled = 1;
            pages_directory[i].p4k.accessed = 0; 
            pages_directory[i].p4k.set_zero = 0; 
            pages_directory[i].p4k.page_size = 0; 
            pages_directory[i].p4k.global_bit = 0; 
            pages_directory[i].p4k.avail = 0;
            pages_directory[i].p4k.base_address = (uint32_t) page_4k_tables >> UPPER_20; // Setting the base address
        } else if (i == 1) { // Setting up kernel space

            pages_directory[i].p4m.present = 1; 
            pages_directory[i].p4m.read_or_write = 1; 
            pages_directory[i].p4m.user_or_supervisor = 0; 
            pages_directory[i].p4m.page_write_through = 0; 
            pages_directory[i].p4m.page_cache_disabled = 0; 
            pages_directory[i].p4m.accessed = 0; 
            pages_directory[i].p4m.dirty = 0; 
            pages_directory[i].p4m.page_size = 1; 
            pages_directory[i].p4m.global_bit = 1; 
            pages_directory[i].p4m.avail = 0;
            pages_directory[i].p4m.page_attribute_table = 0;
            pages_directory[i].p4m.reserved = 0; 
            pages_directory[i].p4m.base_address = 1; 
        } else if (i == VIDEO_INDEX) { // Setting up video mapping
            pages_directory[i].p4k.present = 1;
            pages_directory[i].p4k.read_or_write = 1;
            pages_directory[i].p4k.user_or_supervisor = 1; 
            pages_directory[i].p4k.page_write_through = 0;
            pages_directory[i].p4k.page_cache_disabled = 1;
            pages_directory[i].p4k.accessed = 0; 
            pages_directory[i].p4k.set_zero = 0; 
            pages_directory[i].p4k.page_size = 0; 
            pages_directory[i].p4k.global_bit = 0; 
            pages_directory[i].p4k.avail = 0;
            pages_directory[i].p4k.base_address = ((int32_t)video_map_dir) / FOUR_KB; // Setting the base address
        }
        else { // Setting up everything else
            pages_directory[i].p4m.present = 0;
            pages_directory[i].p4m.read_or_write = 1;
            pages_directory[i].p4k.present = 0;
            pages_directory[i].p4k.read_or_write = 1;

            pages_directory[i].p4m.user_or_supervisor = 0;
            pages_directory[i].p4m.page_write_through = 0;
            pages_directory[i].p4m.page_cache_disabled = 0; 
            pages_directory[i].p4m.accessed = 0; 
            pages_directory[i].p4m.dirty = 0; 
            pages_directory[i].p4m.page_size = 0;
            pages_directory[i].p4m.global_bit = 0; 
            pages_directory[i].p4m.avail = 0;
            pages_directory[i].p4m.page_attribute_table = 0;
            pages_directory[i].p4m.reserved = 0;
            pages_directory[i].p4m.base_address = 0; 
        } 
    }

    // Working with the page tables
    for (i = 0; i < TABLES_NUM; i++) {
        if (i != index && i != (index+1) && i != (index+2) && i != (index+3)) { // For non-video in regular paging
            page_4k_tables[i].present = 0 ;

            page_4k_tables[i].read_or_write = 0; 
            page_4k_tables[i].user_or_supervisor = 0;
            page_4k_tables[i].page_write_through = 0; 
            page_4k_tables[i].page_cache_disabled = 0;
            page_4k_tables[i].accessed = 0; 
            page_4k_tables[i].dirty = 0;
            page_4k_tables[i].page_attribute_table = 0; 
            page_4k_tables[i].avail = 0; 
            page_4k_tables[i].base_address = 0;
        } else { // for video in regular paging
            page_4k_tables[i].present = 1;
            page_4k_tables[i].read_or_write = 1; 
            page_4k_tables[i].user_or_supervisor = 1;
            page_4k_tables[i].page_write_through = 0; 
            page_4k_tables[i].page_cache_disabled = 1;
            page_4k_tables[i].accessed = 0; 
            page_4k_tables[i].dirty = 0;
            page_4k_tables[i].page_attribute_table = 0; 
            page_4k_tables[i].avail = 0x0; 
            page_4k_tables[i].base_address = i; 
        }

        // Working with the video map page table
        if (i != 0) { // For non-video in video mapping
            video_map_dir[i].present = 0 ;

            video_map_dir[i].read_or_write = 0; 
            video_map_dir[i].user_or_supervisor = 0;
            video_map_dir[i].page_write_through = 0; 
            video_map_dir[i].page_cache_disabled = 0;
            video_map_dir[i].accessed = 0; 
            video_map_dir[i].dirty = 0;
            video_map_dir[i].page_attribute_table = 0; 
            video_map_dir[i].avail = 0; 
            video_map_dir[i].base_address = 0;
        } else { // for video in video mapping
            video_map_dir[i].present = 1;
            video_map_dir[i].read_or_write = 1; 
            video_map_dir[i].user_or_supervisor = 1;
            video_map_dir[i].page_write_through = 0; 
            video_map_dir[i].page_cache_disabled = 1;
            video_map_dir[i].accessed = 0; 
            video_map_dir[i].dirty = 0;
            video_map_dir[i].page_attribute_table = 0; 
            video_map_dir[i].avail = 0x0; 
            video_map_dir[i].base_address = VIDEO_MEMS/FOUR_KB; // Setting base address
        }
    }

    // calls the assembly code for paging
    loadPageDirectory((unsigned int*)pages_directory); 
    enablePaging();
}

/*
 * terminal_memory
 *  DESCRIPTION: Sets the 4k tables for the 3 terminals
 *  INPUTS: index - memory location for each terminal
 *  OUTPUTS: None
 *  RETURN VALUES: None
 *  SIDE EFFECTS: Sets the paging for the terminals
 */
void terminal_memory(int index) {
    // 3FF magic number - makes sure that we only get the bits that we want
    page_4k_tables[(index / FOUR_KB) & 0x3FF].present = 1;
    page_4k_tables[(index / FOUR_KB) & 0x3FF].read_or_write = 1; 
    page_4k_tables[(index / FOUR_KB) & 0x3FF].user_or_supervisor = 1;
    page_4k_tables[(index / FOUR_KB) & 0x3FF].page_write_through = 0; 
    page_4k_tables[(index / FOUR_KB) & 0x3FF].page_cache_disabled = 1;
    page_4k_tables[(index / FOUR_KB) & 0x3FF].accessed = 0; 
    page_4k_tables[(index / FOUR_KB) & 0x3FF].dirty = 0;
    page_4k_tables[(index / FOUR_KB) & 0x3FF].page_attribute_table = 0; 
    page_4k_tables[(index / FOUR_KB) & 0x3FF].avail = 0x0; 
    page_4k_tables[(index / FOUR_KB) & 0x3FF].base_address = index / FOUR_KB;
}

