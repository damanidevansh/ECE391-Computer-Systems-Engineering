#include "terminal.h"
#include "keyboard.h"
#include "lib.h"
#include "system_calls.h"

// Initializing variables
int terminal2switch = 0;
int terminal3switch = 0;
extern volatile int cur_frequency;
extern volatile unsigned int terminal_frequency[TERMINAL_NUM];

/*
 * terminal_init
 *  DESCRIPTION: Initializes each of the 3 terminals and the terminals struct
 *  INPUTS: None
 *  OUTPUTS: None
 *  RETURN VALUES: None
 *  SIDE EFFECTS: Sets up the virtual memory for each terminal
 */
int32_t terminal_init() {
    /* initialize all variables that exist within the terminal*/
    int i;
    for (i = 0; i < TERMINAL_NUM; i++){
        terminals[i].buf_pos = 0; // buffer's position
        terminals[i].stdin_enable = 0;
        terminals[i].cursor_x = 0; // position of cursors
        terminals[i].cursor_y = 0;
        terminals[i].video_buff = VIDEO_MEMS + (i+1)*FOUR_KB; // vidmap buffer
        terminals[i].vidmap = 0;
        terminals[i].tid = i;
        terminals[i].curr_pid = -1;
        terminal_memory(terminals[i].video_buff);
    }

    /*flush the TLB*/
    flush_tlb();

    // fill the keyboard buffer
    for (i = 0; i < KB_BUF_SIZE; i++) {
        keyboard_buff[i] = terminals[0].keyboard_buf[i];
    }

    cur_terminal = 0;

    // set screen positions for x and y coord
    ctr = terminals[0].buf_pos;
    screen_x = terminals[0].cursor_x;
    screen_y = terminals[0].cursor_y;
    return 0;
}

/* 
 *  terminal_switch(int32_t new_terminal)
 *  DESCRIPTION: Perform terminal switch, called when alt + f123 is pressed 
 *  INPUTS: The idx of the new terminal
 *  OUTPUTS: Change the content in the terminals array accordingly
 *          also set page for video memory, copy the content out of and into according buffer
 *  RETURN VALUE: Should always return 0 for success
 *  SIDE EFFECTS: Switches to the new terminal
 */
int32_t terminal_switch(int32_t new_terminal){
    /* if swith to current, do nothing */
    if (new_terminal == cur_terminal){
        return 0;
    }

    /* save and update current buffer ptr and cursor position */
    terminal_t* cur_terminal_ptr = &terminals[cur_terminal];
    terminal_t* new_terminal_ptr = &terminals[new_terminal];
    
    /* save current info into structure */
    cur_terminal_ptr->buf_pos = ctr;
    cur_terminal_ptr->cursor_x = screen_x;
    cur_terminal_ptr->cursor_y = screen_y;

    /* restore new info from structure */
    memcpy(keyboard_buff, new_terminal_ptr->keyboard_buf, KB_BUF_SIZE);
    ctr = new_terminal_ptr->buf_pos;

    /* switch current video memory */
    memcpy((void*)(cur_terminal_ptr->video_buff),(void*)VIDEO_MEMS,FOUR_KB);
    memcpy((void*)VIDEO_MEMS,(void*)(new_terminal_ptr->video_buff),FOUR_KB);

    screen_x = new_terminal_ptr->cursor_x;
    screen_y = new_terminal_ptr->cursor_y;
    update_cursor(screen_x,screen_y); // update x and y positions

    int old_terminal = cur_terminal; // update terminal
    terminal_frequency[cur_terminal] = cur_frequency;
    cur_terminal = new_terminal;
    cur_frequency = terminal_frequency[cur_terminal];

    int old_pid = terminals[old_terminal].curr_pid; // update pid
    int new_pid = terminals[new_terminal].curr_pid;

    pcb_t * old_pcb = (pcb_t*)(EIGHT_MB - EIGHT_KB*(old_pid+1)); // update pcb
    pcb_t * new_pcb = (pcb_t*)(EIGHT_MB - EIGHT_KB*(new_pid+1));

    // gets the old ebp and esp values to change processes
    register uint32_t old_ebp asm("ebp");
    register uint32_t old_esp asm("esp");
    old_pcb->saved_ebp = old_ebp;
    old_pcb->saved_esp = old_esp;

    // executes a new shell if switching to terminal 2 for the first time
    if (new_terminal == 1 && terminal2switch == 0) {
        terminal2switch = 1;
        shell_flag = 1;
        execute((const uint8_t*)"shell");
    }

    // executes a new shell if switching to terminal 3 for the first time
    if (new_terminal == 2 && terminal3switch == 0) {
        terminal3switch = 1;
        shell_flag = 1;
        execute((const uint8_t*)"shell");
    }

    uint32_t mem_index = PHYS_MEM_START + new_pid;  // Skip first page
    uint32_t phys_addr = mem_index * PAGE_4MB;      // Physical address
    
    // Initializing paging. Setting all required bits
    pages_directory[USER_INDEX].p4m.base_address = phys_addr >> 22; // 22 MAGIC NUMBER- 22 bit addressability
    flush_tlb();

    tss.esp0 = (EIGHT_MB-((new_pid)*EIGHT_KB)-4);   // 4 magic number - sets the value for the context switcH

    int updated_esp = new_pcb->saved_esp; // update base and stack pointers
    int updated_ebp = new_pcb->saved_ebp;

    // moves the values back to the updated esp and ebp pointers
    asm volatile("\
        movl %0, %%esp; \
        movl %1, %%ebp;"
        :
        : "r"(updated_esp), "r"(updated_ebp)
        : "%esp", "%ebp"
    );
    
    return 0;
}

/*
 * terminal_open
 *  DESCRIPTION: This function opens the terminal
 *  INPUTS: filename - takes in the filename to open
 *  OUPUTS: None
 *  RETURN VALUE: 0 - always returns 0
 *  SIDE EFFECTS: None
 */
int terminal_open(const uint8_t* filename){
    return 0;
}

/*
 * terminal_close
 *  DESCRIPTION: This function closes the terminal
 *  INPUTS: fd - file descriptor number
 *  OUTPUTS: None
 *  RETURN VALUE: 0 - always returns 0
 *  SIDE EFFECTS: None
 */
int terminal_close(int32_t fd){
    return 0;
}

/*
 * terminal_read
 *  DESCRIPTION: Reads data from the keyboard buffer and puts it into the buffer
 *  INPUTS: fd - file descriptor number
 *          buffer - the buffer we read into
 *          nbytes - number of bytes
 *  OUTPUTS: None
 *  RETURN VALUE: -1 for failure, ctr for success
 *  SIDE EFFECTS: None
 */
int terminal_read(int32_t fd, void* buffer, int32_t nbytes){
    sti();    
    int i;            
    ctr = 0; // set flags
    flag_enter = 0;
    while(1){
        cli(); // mask interrupt
        /* series of error checks */
        if (buffer == NULL) {
            return -1;
        }

        if (nbytes < 0) {
            printf("Invalid size! \n");
            return -1;
        }

        // Setting last character of the buffer to a newline
        tempkeyboard_buff[nbytes] = '\n';

        // Break out of while loop when enter is pressed
        if(flag_enter == 1) {
            break;
        }
        
        sti(); // set flags
    }

    /* copies the buffer from the keyboard buffer to the buffer */
    for (i = 0; i < nbytes+1; i++) {
        ((char*)buffer)[i] = tempkeyboard_buff[i];
    }

    /* resets the tempkeyboard_buff */
    memset(tempkeyboard_buff, 0, KB_BUF_SIZE);

    return (ctr + 1);   // have to return plus 1 due to newline character at the end of the buffer
}

/*
 * terminal_write
 *  DESCRIPTION: This function takes the buffer and writes it to the screen
 *  INPUTS: fd - file descriptor number
 *          buffer - buffer that we write to the screen
 *          nbytes - number of bytes
 *  OUTPUTS: None
 *  RETURN VALUE: -1 for failure, 0 for success
 *  SIDE EFFECTS: None
 */
int terminal_write(int32_t fd, const void* buffer, int32_t nbytes){
    int i;
    
    /* series of error checks */
    if (buffer == NULL) {
        return -1;
    }

    if (nbytes < 0) {
        printf("Invalid size! \n");
        return -1;
    }
    
    // PUTCing all the characters from the buffer for nbytes characters
    for(i = 0; i < nbytes; ++i){
        putc(((char*)buffer)[i]);
    }

    return 0;
}

