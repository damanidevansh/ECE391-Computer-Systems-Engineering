#include "system_calls.h"
#include "terminal.h"

// stop_keyboard = 0;
/* initialize pid variables and arrays */
uint32_t cur_pid = 0;  
uint32_t parent_pid = 0;
uint32_t pid_array[PID_MAX];
int shellpid[TERMINAL_NUM];

/* file operation table variables*/
fop_table_t stdin_fop; 
fop_table_t stdout_fop;
fop_table_t rtc_fop; 
fop_table_t dir_fop;
fop_table_t file_fop;
fop_table_t null_fop;
/*
 * please work
 *  DESCRIPTION: sets up the file operating table for the required file operators
 *  INPUTS: None
 *  OUTPUTS: None
 *  RETURN VALUE: None
 *  SIDE EFFECTS: initializes the fop to the requried open/close/read/write functions
 *                per file op.
 */
void please_work() {
  /* sets the fop table for the stdin file directory */
  stdin_fop.read = terminal_read;
  stdin_fop.write = nulls;
  stdin_fop.open = terminal_open;
  stdin_fop.close = terminal_close;

  /* sets the fop table for the stdout file directory */
  stdout_fop.read = nulls;
  stdout_fop.write = terminal_write;
  stdout_fop.open = terminal_open;
  stdout_fop.close = terminal_close;
  
  /* sets the fop table for the rtc file directory */
  rtc_fop.read = rtc_read;
  rtc_fop.write = rtc_write;
  rtc_fop.open = rtc_open;
  rtc_fop.close = rtc_close;

  /* sets the fop table for the file system directory fd */
  dir_fop.read = dir_read;
  dir_fop.write = dir_write;
  dir_fop.open = dir_open;
  dir_fop.close = dir_close;

  /* sets the fop table for the file system file read, write, open, close fd */
  file_fop.read = file_read;
  file_fop.write = file_write;
  file_fop.open = file_open;
  file_fop.close = file_close;

  /* sets the fop table for every other file directory */
  null_fop.read = nulls;
  null_fop.write = nulls;
  null_fop.open = nulls;
  null_fop.close = nulls;

  shellpid[0] = -1;
  shellpid[1] = -1;
  shellpid[2] = -1;
  
}

void halt_assembly(uint32_t ebp, uint32_t esp, uint8_t status); // Halt assembly helper function initialization

/*
 * halt
 *  DESCRIPTION: Halts the currently executing system call
 *  INPUTS: the status of the current system
 *  OUTPUTS: None
 *  RETURN VALUE: -1
 *  SIDE EFFECTS: NONE
 */
int32_t halt(uint32_t status) {
  // Initializing variables
  int i;
  uint32_t esparg = 0;
  uint32_t eiparg = 0;


  // Obtaining pcb pointer
  pcb_t* pcb_ptr = get_pcb(terminals[cur_terminal].curr_pid);

  // If PID is zero- base shell
  if (pcb_ptr->pid == shellpid[0] || pcb_ptr->pid == shellpid[1] || pcb_ptr->pid == shellpid[2]) {
    eiparg = pcb_ptr->user_eip; // Setting EIP to user_eip
    esparg = pcb_ptr->user_esp; // setting esp to user_esp
    asm volatile ("\
      pushl   %1           ;\
      pushl   %3           ;\
      pushfl               ;\
      pushl   %2           ;\
      pushl   %0           ;\
      iret                 ;\
      "
      :
      : "g"(eiparg), "g"(USER_DS), "g"(USER_CS), "g"(esparg)
      : "memory"
    );
  }
 
 // Updating PID
  pcb_t* parent_pcb = get_pcb(pcb_ptr->parent_pid);
  cur_pid = pcb_ptr->parent_pid;
  terminals[cur_terminal].curr_pid = cur_pid;
  pid_array[pcb_ptr->pid] = 0;

  // Restoring Paging stuff
  uint32_t physical_address = (PHYS_MEM_START + cur_pid) *PAGE_4MB;
  pages_directory[USER_INDEX].p4k.base_address = physical_address/FOUR_KB;
  flush_tlb();

  // Closing FDs
  for (i = 0; i < MAX_FD_NUM; i++){
    pcb_ptr->fd_array[i].flag = 0;
  }

  // setting TSS values
  tss.ss0 = KERNEL_DS;
  tss.esp0 = EIGHT_MB - EIGHT_KB*(parent_pcb->pid) - sizeof(uint32_t);


  halt_assembly(pcb_ptr->exec_ebp, pcb_ptr->exec_esp, status);

  return -1;
}

/*
 * execute
 *  DESCRIPTION: executes the called system call
 *  INPUTS: None
 *  OUTPUTS: None
 *  RETURN VALUE: File descriptor upon pass, -1 upon fail
 *  SIDE EFFECTS: none
 */
int32_t execute(const uint8_t* command) {
  int i, j;             //Loop variable
  int cmd_start = 0;    //file_cmd first index
  int arg_start = 0;
  int blank_count = 0;


  //Variables for getting file name and args from command
  int length = strlen((const int8_t*)command);
  int file_cmd_length = 0;

  //File system variables
  uint8_t file_cmd[FILE_BOUND];
  uint8_t file_arg[FILE_BOUND];
  uint8_t final_cmd[FILE_BOUND];
    
  dentry_t temp_dentry;
  uint8_t elf_buf[sizeof(int32_t)];

  //Arguments for switching to user context.
  uint32_t eip_arg;
  uint32_t esp_arg;


// Parse through the command
  cli();

  for (i = 0; i < FILE_BOUND; i++){
    file_cmd[i] = '\0';
    file_arg[i] = '\0';
  }

  // gets the cmd from the input command
  for(i = 0; i < length; i++){
    //Gets the args and file name from the command word.
    if(command[i] != ' '){
      file_cmd[cmd_start] = command[i];
      ++file_cmd_length;
      ++cmd_start;
    } else{
      ++blank_count;
      if(file_cmd_length > 0)
        break;
    }
  }

  for (i = 0; i < FILE_BOUND; i++) final_cmd[i] = file_cmd[i];

  // gets the argument from the input command for the executable
  for(i = cmd_start + blank_count; i < length; i++){
    if(command[i] != ' '){
      for(j = i; j < length; j++){
        file_arg[arg_start] = command[j];
        ++arg_start;            
      }

      break;
    }
  }

// checks file validity
  if(read_dentry_by_name(final_cmd, &temp_dentry) == -1){
    return -1; /* file does not exist*/
  }

  if(read_data(temp_dentry.inode_num, 0, elf_buf, sizeof(int32_t)) == -1){
    return -1;  /* something failed during read_data*/
  }

  if(elf_buf[0] != MAGIC0 || elf_buf[1] != MAGIC1 || elf_buf[2] != MAGIC2 || elf_buf[3] != MAGIC3) {
    return -1; /* ELF not found, not an executable*/
  }

  // sets paging
    pcb_t* pcb_ptr;
    int pid_flag = 0;
    for(i = 0; i < MAX_PID ; i++){         /* find available pid, max 6 for now */
      if(pid_array[i] == 0){
        pid_array[i] = 1;
        cur_pid = i;                  /* set cur_pid to new one*/

        pid_flag = 1;    
        break;
      }
    }

    /* updates the curr_pid for each terminal */
    int oldpid = terminals[cur_terminal].curr_pid;
    terminals[cur_terminal].curr_pid = cur_pid;
    if(shell_flag)
    {
      shellpid[shell_ctr] = cur_pid;
      shell_ctr++;
      shell_flag = 0;
    }

    if(pid_flag == 0){
      return -1;
    }

    // Calculating physical address
    uint32_t mem_index = PHYS_MEM_START + cur_pid;  // Skip first page
    uint32_t phys_addr = mem_index * PAGE_4MB;      // Physical address
    
    // Initializing paging. Setting all required bits
    pages_directory[USER_INDEX].p4m.page_size = 1;
    pages_directory[USER_INDEX].p4m.base_address = phys_addr >> 22; // 22 MAGIC NUMBER- 22 bit addressability
    pages_directory[USER_INDEX].p4m.global_bit = 0;
    pages_directory[USER_INDEX].p4m.page_cache_disabled = 1;
    pages_directory[USER_INDEX].p4m.user_or_supervisor = 1;
    pages_directory[USER_INDEX].p4m.present = 1;
    flush_tlb();


// loads the file into memory
    inode_t* temp_inode_ptr = (inode_t*)(ptr_inode + temp_dentry.inode_num);
    uint8_t* image_addr = (uint8_t*)PROGRAM_IMAGE_ADDR;           //it should stay the same, overwriting existing program image
    read_data(temp_dentry.inode_num, (uint32_t)0, image_addr,temp_inode_ptr->length); //copying entire file to memory starting at Virt addr 0x08048000

// creates the pcb and opens the fd
    pcb_ptr = get_cur_pcb();          /* create PCB - maybe use the other get_pcb function      */
    pcb_ptr->pid = cur_pid;
    pcb_ptr->parent_pid = oldpid;



    // Initialize the file descriptor array
    for (i = 2; i < MAX_NUM_FILE; i++) {
      pcb_ptr->fd_array[i].fop_table_ptr = &null_fop;
      pcb_ptr->fd_array[i].inode = 0;
      pcb_ptr->fd_array[i].file_pos = INIT_FILE_POS;
      pcb_ptr->fd_array[i].flag = FD_OUT;
    }

    pcb_ptr->fd_array[0].fop_table_ptr = &stdin_fop;        /* stdin  */
    pcb_ptr->fd_array[0].inode = 0;
    pcb_ptr->fd_array[0].file_pos = INIT_FILE_POS;
    pcb_ptr->fd_array[0].flag = FD_IN;

    pcb_ptr->fd_array[1].fop_table_ptr = &stdout_fop;       /* stdout */
    pcb_ptr->fd_array[1].inode = 0;
    pcb_ptr->fd_array[1].file_pos = INIT_FILE_POS;
    pcb_ptr->fd_array[1].flag = FD_IN;

    for (i = 0; i < 1024; i++) { // 1024- size of command argument
      pcb_ptr->cmd_arg[i] = 0;
    }

    strncpy((int8_t*)pcb_ptr->cmd_arg, (int8_t*)(file_arg), arg_start);

  // context switch
    //Bytes 24 to 27 of the executable. entry point
    uint8_t eip_buf[ELF_SIZE];
    read_data(temp_dentry.inode_num, ELF_START, eip_buf, sizeof(int32_t)); // Read eip from elf (location 24)
    eip_arg = *((int*)eip_buf);

    esp_arg = USER_MEM + PAGE_4MB - sizeof(int32_t); // where program starts

    pcb_ptr->user_eip = eip_arg;
    pcb_ptr->user_esp = esp_arg;

    //For privilege level switch
    tss.ss0 = KERNEL_DS;
    tss.esp0 = EIGHT_MB - (EIGHT_KB*cur_pid) - sizeof(int32_t);
    pcb_ptr->tss_esp0 = tss.esp0;

    //Get the esp and ebp values for the user context switch.
    uint32_t esp;
    uint32_t ebp;
    asm("\t movl %%esp, %0" : "=r"(esp));
    asm("\t movl %%ebp, %0" : "=r"(ebp));
    pcb_ptr->exec_esp = esp;
    pcb_ptr->exec_ebp = ebp;

  // IRET stuff
  // eax = eip_arg, ebx = USER_DS, ecx = USER_CS, edx = esp_arg
    asm volatile ("\
        pushl   %1           ;\
        pushl   %3           ;\
        pushfl               ;\
        movl (%%esp), %%eax  ;\
        orl $0x0200, %%eax   ;\
        movl %%eax, (%%esp)    ;\
        pushl   %2           ;\
        pushl   %0           ;\
        iret                 ;\
        "
        :
        : "g"(eip_arg), "g"(USER_DS), "g"(USER_CS), "g"(esp_arg)
        : "memory"
    );

    return 0;
}


/*
 * read
 *  DESCRIPTION: reads the required file 
 *  INPUTS: file descriptor, buffer to read from and number of bytes to read
 *  OUTPUTS: None
 *  RETURN VALUE: -1 upon fail, pointer to read fop of the required fd
 *  SIDE EFFECTS: None
 */
int32_t read(int32_t fd, void* buf, int32_t nbytes) {
  pcb_t* cur_pcb_ptr_read = get_cur_pcb(); // Obtaining PCB

  // NULL CHECKS
  if(fd < 0 || fd >= MAX_NUM_FILE){
    return -1;
  }
  
  if (buf == NULL){
    return -1;
  }

  if (nbytes < 0){
    return -1;
  }    

  if(cur_pcb_ptr_read->fd_array[fd].flag == FD_OUT) { // Return -1 if closed
    return -1;
  }
  
  return cur_pcb_ptr_read->fd_array[fd].fop_table_ptr->read(fd,buf,nbytes);
}

/*
 * write
 *  DESCRIPTION: write the required file 
 *  INPUTS: file descriptor, buffer to read from and number of bytes to read
 *  OUTPUTS: None
 *  RETURN VALUE: -1 upon fail, pointer to write fop of the required fd
 *  SIDE EFFECTS: None
 */
int32_t write(int32_t fd, const void* buf, int32_t nbytes) {
  pcb_t* cur_pcb_ptr_write = get_cur_pcb(); // Obtaining PCB

  // NULL CHECKS
  if(fd < 0 || fd > MAX_NUM_FILE){
    return -1;
  }
    
  if (buf == NULL){
    return -1;
  }
    
  if (nbytes < 0){
    return -1;
  }

  if(cur_pcb_ptr_write->fd_array[fd].flag == 1) { // Return -1 if free
    return -1;
  }

  return cur_pcb_ptr_write->fd_array[fd].fop_table_ptr->write(fd,buf,nbytes);
}

/*
 * open
 *  DESCRIPTION: opens the required file
 *  INPUTS: filename
 *  OUTPUTS: None
 *  RETURN VALUE: -1 upon fail, the current fd upon pass
 *  SIDE EFFECTS: None
 */
int32_t open(const uint8_t* filename) {
  int i;  // Looping variable
  dentry_t dentry; // Initializing dentry

  // NULL CHECKS
  if(strlen((char*)filename) == NULL){
      return -1;
  }

  if (strlen((char*)filename) < 0 || strlen((char*)filename) > 32) { // 32- MAX FILE SIZE
    return -1;
  }

  if(read_dentry_by_name(filename, &dentry) == -1){
      return -1;
  }

  pcb_t* pcb_ptr = get_cur_pcb(); // obtains the PCB

  // Setting fd information
  for (i = BEGIN_FILE_DESC; i < MAX_FD_NUM; i++) {
    if(pcb_ptr->fd_array[i].flag == 1) { // IF FREE
      pcb_ptr->fd_array[i].file_pos = INIT_FILE_POS;
      pcb_ptr->fd_array[i].inode = dentry.inode_num;
      pcb_ptr->fd_array[i].flag = 0; // SET TO BUSY

      if (dentry.filetype == 0) { // RTC
        pcb_ptr->fd_array[i].fop_table_ptr = &rtc_fop;
      } else if (dentry.filetype == 1){ // Directory
        pcb_ptr->fd_array[i].fop_table_ptr = &dir_fop;
      } else if (dentry.filetype == 2) { // Regular file
        pcb_ptr->fd_array[i].fop_table_ptr = &file_fop;
      }
      return i; // if pass, return file descriptor
    }
  }

  return -1; // If failure
}

/*
 * close
 *  DESCRIPTION: Closes the required file
 *  INPUTS: fd - file descriptor number to close
 *  OUTPUTS: None
 *  RETURN VALUE: -1 for invalid conditions, pointer to close fop of the required fd
 *  SIDE EFFECTS: None
 */
int32_t close(int32_t fd) {
  // get pointer to the pcb
  pcb_t* pcb_ptr = get_cur_pcb();
  
  // invalid fd check
  if(fd < BEGIN_FILE_DESC || fd > MAX_NUM_FILE || pcb_ptr->fd_array[fd].flag == FD_OUT){
    return -1; 
  }

  // Closing required fds and freeing inodes
  pcb_ptr->fd_array[fd].flag = 1; // Set to free
  pcb_ptr->fd_array[fd].file_pos = INIT_FILE_POS;
  pcb_ptr->fd_array[fd].inode = 0;

  return pcb_ptr->fd_array[fd].fop_table_ptr->close(fd);
}

/*
 * getargs
 *  DESCRIPTION: Reads the program's command-line argument to a user-level buffer
 *  INPUTS: buf- the buffer to copy into. nbytes- number of bytes to read and copy into the buffer
 *  OUTPUTS: None
 *  RETURN VALUE: 0 upon pass, -1 if fail.
 *  SIDE EFFECTS: Copies nbytes of the argument into the buffer
 */
int32_t getargs(uint8_t* buf, int32_t nbytes) {
  // NULL CHECKS
  if (buf == NULL || nbytes < 0) {
    return -1;
  }

  pcb_t* pcb_ptr = get_cur_pcb(); // Obtaining PCB pointer
  if (pcb_ptr->cmd_arg == NULL) { // Check to see if argument is NULL. Fail if it does
    return -1;
  }

  // NULL CHECK for whether the first input of the argument is a NULL
  if(pcb_ptr->cmd_arg[0] == 0){
    return -1;
  }

  // Copy nbytes of argument to the buffer
  int i;
  for (i = 0; i < nbytes; i++) {
    buf[i] = pcb_ptr->cmd_arg[i];
  }
  return 0; // Return pass
}

/*
 * vidmap
 *  DESCRIPTION: Maps the text-mode video memory into user space at a pre-set virtual address 
 *  INPUTS: The memory location to start writing to (uint8_t** screen_start)
 *  OUTPUTS: None
 *  RETURN VALUE: 0 upon pass, -1 if fail.
 *  SIDE EFFECTS: Lets us change between frames and make Mr. Fishy work
 */
int32_t vidmap(uint8_t** screen_start) {
  // NULL CHECKS
  if (screen_start == NULL){
    return -1;
  }

  if ((uint32_t)screen_start < USER_MEM || (uint32_t)screen_start > (USER_MEM+PAGE_4MB)){
    return -1;
  }

  flush_tlb(); // Flush tlb

  *screen_start = (uint8_t*)VIRTUAL_MEM_VIDEO; // Set pointer to screen_start

  return 0;
}

int32_t set_handler(int32_t signum, void* handler_address) {
  return -1;
}

int32_t sigreturn(void) {
  return -1;
}

/* HELPER FUNCTIONS */
/*
 * get_cur_pcb
 *  DESCRIPTION: gets the current pcb based on the cur_pid global variable value
 *  INPUTS: None
 *  OUTPUTS: None
 *  RETURN VALUE: A pointer to the current pcb
 *  SIDE EFFECTS: None
 */
pcb_t* get_cur_pcb(){
  return (pcb_t*)(EIGHT_MB - EIGHT_KB*(terminals[cur_terminal].curr_pid +1));
}

/*
 * get_pcb
 *  DESCRIPTION: gets the pcb that is specified by the pid input
 *  INPUTS: pid - input pid value to get the corresponding pcb
 *  OUTPUTS: None
 *  RETURN VALUE: A pointer to the wanted pcb
 *  SIDE EFFECTS: None
 */
pcb_t* get_pcb(uint32_t pid){
  return (pcb_t*)(EIGHT_MB - EIGHT_KB*(pid+1));
}

/*
 * nulls
 *  DESCRIPTION: This function is used for the file operations table
 *  INPUTS: None
 *  OUTPUTS: None
 *  RETURN VALUE: -1 to show the return value of null values in the file operations table
 *  SIDE EFFECTS: None
 */
int32_t nulls(){
    return -1;
}
