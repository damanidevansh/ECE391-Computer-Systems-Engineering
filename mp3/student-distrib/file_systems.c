#include "file_systems.h"
#include "system_calls.h"

/*
 * file_system_init
 *  DESCRIPTION: Initializes the file system boot block, dentries and inodes
 *  INPUTS: files_input- the start of the module
 *  OUTPUTS: None
 *  RETURN VALUE: 
 *  SIDE EFFECTS: Boots up the file system
 */
void file_system_init(uint32_t files_input) {
  
    ptr_boot_block = (boot_block_t*)(files_input); // Setting up a pointer to the boot block at the starting memory

    // Setting up required boot block entries
    inodes = ptr_boot_block->inodes_number;
    data_blocks_total = ptr_boot_block->data_blocks_number;
    dentries = ptr_boot_block->dentries_number;

    // Setting up required starting base pointers for the inodes, data and dentries
    ptr_inode = (inode_t*)(ptr_boot_block + INODE_FIND);
    ptr_data = (uint8_t*)(ptr_inode + inodes); 
    ptr_dentry = ptr_boot_block->dentry;
}

/*
 * read_dentry_by_name
 *  DESCRIPTION: Reads the inputted dentry and searches for the required filename
 *  INPUTS: file_name (the name of the file), dentry (in which we have to search)
 *  OUTPUTS: None
 *  RETURN VALUE: 0 upon pass, -1 upon fail.
 *  SIDE EFFECTS: Obtains the required file from the given dentry
 */ 
int32_t read_dentry_by_name (const uint8_t* file_name, dentry_t* dentry){

  // Initializing variables
  int i;
  int fname_length;
  int length;
  length = strlen((int8_t*) file_name);

  /* loops through all the dentries */
  for (i = 0; i < DENTRY_NUM; i++) {
    if (strlen((int8_t*)ptr_dentry[i].filename) > FILE_BOUND) { // If file name is greater than its max length of 32
      fname_length = FILE_BOUND; // Manually set it to 32
    } else {
      fname_length = strlen((int8_t*)ptr_dentry[i].filename); // Else, get its length
    }

    // if inputted of filename is the same as that in the given dentry, we want to get into this loop
    if (length == fname_length) {
      if ((strncmp((int8_t*) file_name, (int8_t*)ptr_dentry[i].filename, FILE_BOUND)) == 0) { // if file name is the same as dentry's fname
        read_dentry_by_index(i, dentry); // call read dentry by index
        return 0;
      }
    }
  }

  return -1;
}

/*
 * read_dentry_by_index
 *  DESCRIPTION: Reads the inputted dentry and searches for the required index
 *  INPUTS: index (for which we need to search), dentry (in which we have to search)
 *  OUTPUTS: None
 *  RETURN VALUE: 0 upon pass, -1 upon fail
 *  SIDE EFFECTS: Obtains whatever's in the inputted dentry's inputted index value
 */
int32_t read_dentry_by_index (uint8_t index, dentry_t* dentry){
  /* if the index isn't in the boot block */
  if(index >= dentries) { 
    return -1;
  }

  /* copies all dentry information into new dentry at specified index */
  strncpy((int8_t*)dentry->filename, (int8_t*)ptr_dentry[index].filename, FILE_BOUND);
  dentry->inode_num = ptr_dentry[index].inode_num;
  dentry->filetype = ptr_dentry[index].filetype;
  
  return 0;
}

/*
 * read_data
 *  DESCRIPTION: Reads data from a given file input
 *  INPUTS: Inode number, offset to read from, buffer to read from and length of bytes to read
 *  OUTPUTS: None
 *  RETURN VALUE: the number of bytes read (i) upon pass, 0 if the offset is greater than the inode length, -1 if failed elsewhere
 *  SIDE EFFECTS: None
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    // Initializing variables
    int i;
    inode_t* current_inode_block = (inode_t*)(ptr_inode + inode); 
    uint32_t inode_length = current_inode_block->length;                                                      
    uint32_t data_blocks = offset / INODE_BLOCK_SIZE;      
    uint32_t data_block_index = offset % INODE_BLOCK_SIZE;           
    uint32_t* ptr_inode_block;                                 

    /* bound checks */
    if (offset >= inode_length) {                
      return 0; 
    }
    if (inode >= inodes || inode < 0) {        
      return -1;
    }

    /* iterates through each byte of data and stores it into buf */
    for (i = 0; i < length; i++, buf++) {

      if (offset + i >= inode_length) { // if offset plus whatever bit we're reading is at the end of the inode, return
        return i;
      }

      if (current_inode_block->data_blocks_num[data_blocks] >= data_blocks_total) { // if the block # is greater than the total number of blocks, ret -1
        return -1;
      }
      
      // updating inode block pointer information
      ptr_inode_block = (uint32_t*)(ptr_data + (INODE_BLOCK_SIZE)*(current_inode_block->data_blocks_num[data_blocks]) + data_block_index);

      data_block_index++; // incrementing the number of data block we're reading from

      // check to see if data block index is out of bounds
      if (data_block_index >= INODE_BLOCK_SIZE) {
        data_block_index = 0;
        data_blocks++;
      }

      // Copy the inode block into the buffer for that byte
      memcpy(buf, ptr_inode_block, 1);
    }

    return i; // return numbytes read upon success
}

/*
 * file_open
 *  DESCRIPTION: Opens the required file
 *  INPUTS: the file's name file_name
 *  OUTPUTS: None
 *  RETURN VALUE: 0 upon pass, -1 upon fail
 *  SIDE EFFECTS: None
 */
int32_t file_open(const uint8_t* file_name){
  if (file_name == NULL) { // if there is no file, return -1
    return -1;
  }

  return 0;
}

/*
 * file_close
 *  DESCRIPTION: Close the file
 *  INPUTS: int32_t fd - file directory
 *  OUTPUTS: None
 *  RETURN VALUE: Return -1 on fail, return 0 on success
 *  SIDE EFFECTS: None
 */
int32_t file_close(int32_t fd){
  // NULL CHECKS
  if (fd == 0 || fd == 1 || fd > 7) { // fd bounds check, can't have a file descriptor number of 0, 1 or 7
    return -1 ;
  }

  return 0;
}

/*
 * file_read
 *  DESCRIPTION: read the file
 *  INPUTS: int32_t fd - file directory, const void* buf - buffer to read from, int32_t nbytes - number of bytes
 *  OUTPUTS: None
 *  RETURN VALUE: Return -1 on fail, return bytes read on success
 *  SIDE EFFECTS: None
 */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes){
  if (buf == NULL) {// NULL check
    return -1;
  } 

  if (fd == 0 || fd == 1 || fd > 7) { // fd bounds check, can't have a file descriptor number of 0, 1 or 7
    return -1 ;
  }

  // Obtaining current PCB pointer and getting the number of bytes we need to read by calling read_data()
  pcb_t* pcb_ptr = get_cur_pcb();
  uint32_t bytes_read = read_data(pcb_ptr->fd_array[fd].inode, pcb_ptr->fd_array[fd].file_pos, buf, nbytes);
  
  /* NULL CHECKS */
  if(bytes_read == -1) {
    return -1;
  }

  pcb_ptr->fd_array[fd].file_pos += bytes_read; // Incrementing file position by the number of bytes read
  return bytes_read;
}

/*
 * file_write
 *  DESCRIPTION: write to the file
 *  INPUTS: int32_t fd - file directory, const void* buf - buffer to read from, int32_t nbytes - number of bytes
 *  OUTPUTS: None
 *  RETURN VALUE: Return -1
 *  SIDE EFFECTS: None
 */
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes){
  if (buf == NULL || nbytes == NULL) { // NULL check
    return -1;
  }
        
  if (fd == 0 || fd == 1 || fd > 7) { // fd bounds check, can't have a file descriptor number of 0, 1 or 7
    return -1 ;
  }

  return -1;
}

/*
 * dir_open
 *  DESCRIPTION: Open the directory
 *  INPUTS: const uint8_t* file_name - name of the file
 *  OUTPUTS: None
 *  RETURN VALUE: Return -1 on fail, return 0 on success
 *  SIDE EFFECTS: None
 */
int32_t dir_open(const uint8_t* file_name){
  if (file_name == NULL) { // if no file, return -1
    return -1;
  }

  return 0;
}

/*
 * dir_close
 *  DESCRIPTION: Close the directory 
 *  INPUTS: int32_t fd - file directory
 *  OUTPUTS: None
 *  RETURN VALUE: Return -1 on fail, return 0 on success
 *  SIDE EFFECTS: None
 */
int32_t dir_close(int32_t fd){
  if (fd == 0 || fd == 1 || fd > 7) { // fd bounds check, can't have a file descriptor number of 0, 1 or 7
    return -1 ;
  }

  return 0;
} 

/*
 * dir_read
 *  DESCRIPTION: Read the directory
 *  INPUTS: int32_t fd - file directory, const void* buf - buffer to read from, int32_t nbytes - number of bytes
 *  OUTPUTS: None
 *  RETURN VALUE: return 0 if fail, return nbytes on success
 *  SIDE EFFECTS: None
 */
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes){
  dentry_t dentry;
  pcb_t* pcb_ptr = get_cur_pcb(); // get pointer to the pcb
  int32_t curr_pos = pcb_ptr->fd_array[fd].file_pos; // access file pos

  /* NULL CHECKS */
  if (read_dentry_by_index(curr_pos, &dentry) == -1) {
    return 0;
  }

  int32_t num_bytes = strlen((int8_t*)&(dentry.filename)); // getting the number of bytes we need to read by checking length of the filename
  strncpy((int8_t*)buf, (int8_t*)&(dentry.filename), FILE_BOUND); // copying the file information into the inputted buffer
  
  curr_pos++; // Incrementing current position and setting it to the file position
  pcb_ptr->fd_array[fd].file_pos = curr_pos;

  /* NULL CHECKS */
  if (num_bytes > FILE_BOUND) {
    num_bytes = FILE_BOUND;
  }

  if(curr_pos > dentries){
    return 0;
  }

  return num_bytes; // return nbytes
}

/*
 * dir_write
 *  DESCRIPTION: Write to the file directory
 *  INPUTS: int32_t fd - file directory, const void* buf - buffer to read from, int32_t nbytes - number of bytes
 *  OUTPUTS: None
 *  RETURN VALUE: Return -1
 *  SIDE EFFECTS: None
 */
int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes) {
  if (buf == NULL || nbytes == NULL) { // NULL Check
    return -1;
  } 
        
  if (fd == 0 || fd == 1 || fd > 7) { // fd bounds check, can't have a file descriptor number of 0, 1 or 7
    return -1 ;
  }

  return -1; 
}
