#ifndef SYSTEM_CALLS_H
#define SYSTEM_CALLS_H

#include "types.h"
#include "lib.h"
#include "file_systems.h"
#include "terminal.h"
#include "paging.h"
#include "rtc.h"
#include "x86_desc.h"
#include "idt.h"

// magic numbers for the executable
#define MAGIC0    0x7f
#define MAGIC1    0x45
#define MAGIC2    0x4c
#define MAGIC3    0x46

#define MAX_FD_NUM 8
#define PID_MAX   8
#define MAX_PID   6
#define MAX_NUM_FILE 8      // maximum number of open files
#define INIT_FILE_POS 0
#define FD_OUT 1
#define FD_IN 0
#define ELF_SIZE        4
#define ELF_START       24
#define USER_MEM      0x8000000 //Page address for user stack

#define PHYS_MEM_START  2
#define PAGE_4MB        0x400000  
#define PROGRAM_IMAGE_ADDR 0x08048000
#define USER_INDEX      32        

#define EIGHT_MB 0x800000
#define EIGHT_KB 0x2000

#define VIRTUAL_MEM_VIDEO 0x8800000

// Initializing File Operator Table Struct
typedef struct {
    int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
    int32_t (*open)(const uint8_t* fname);
    int32_t (*close)(int32_t fd);
} fop_table_t;

// Initializing File Descriptor Table Struct
typedef struct {
    fop_table_t* fop_table_ptr;
    uint32_t inode;
    uint32_t file_pos;
    uint32_t flag;
} file_descriptor_t;

// Initializing PCB Struct
typedef struct{
    file_descriptor_t fd_array[MAX_FD_NUM];
    uint32_t pid;
    uint32_t parent_pid;
    uint32_t exec_esp;
    uint32_t exec_ebp;
    uint32_t tss_esp0;
    uint32_t user_eip;
    uint32_t user_esp;
    uint32_t saved_esp;
    uint32_t saved_ebp;

    uint8_t cmd_arg[1024];
} pcb_t;

int shell_flag;
int shell_ctr;

// initializes the fop table and .S functions
extern void system_call_handler();
extern void flush_tlb();
extern void please_work();

// all the system calls for 3.3
int32_t halt(uint32_t status);
int32_t execute(const uint8_t* command);
int32_t read(int32_t fd, void* buf, int32_t nbytes);
int32_t write(int32_t fd, const void* buf, int32_t nbytes);
int32_t open(const uint8_t* filename);
int32_t close(int32_t fd);

// all the system calls for 3.4
int32_t getargs(uint8_t* buf, int32_t nbytes);
int32_t vidmap(uint8_t** screen_start);
int32_t set_handler(int32_t signum, void* handler_address);
int32_t sigreturn(void);

// helper functions
pcb_t* get_cur_pcb();
pcb_t* get_pcb(uint32_t pid);
int32_t nulls();


#endif
