#ifndef TERMINAL_H
#define TERMINAL_H

#include "keyboard.h"
#include "types.h"
#include "paging.h"
#include "system_calls.h"
#include "lib.h"
#include "pit.h"

#define TERMINAL_NUM 3

// Terminal Struct
typedef struct terminal_t{
    
    char keyboard_buf[128]; // keyboard buffer
    volatile uint8_t stdin_enable;
    uint8_t buf_pos;
    int cursor_x; // cursor positions
    int cursor_y;
    uint32_t video_buff; // video mem buffer

    /* parameters used for multi-processes control */
    int32_t curr_pid;
    
    struct terminal_t* next_terminal;
    int32_t tid;
    int32_t vidmap;
    int count;
} terminal_t;

volatile int32_t cur_terminal;

/* This function opens the terminal */
int terminal_read(int32_t fd, void* buffer, int32_t nbytes);

/* This function closes the terminal */
int terminal_write(int32_t fd, const void* buffer, int32_t nbytes);

/* Reads data from the keyboard buffer and puts it into the buffer */
int terminal_open(const uint8_t* filename);

/* This function takes the buffer and writes it to the screen */
int terminal_close(int32_t fd);

/* Initializes each of the 3 terminals and the terminals struct */
int32_t terminal_init();

/* swith the terminal called in keyboard interrupt */
int32_t terminal_switch(int32_t new_terminal);

terminal_t terminals[TERMINAL_NUM];


#endif
