 #include "keyboard.h"
 #include "lib.h"
 #include "system_calls.h"
 #include "rtc.h"

/* list of all the scan codes for each keyboard button */
char scan_codes[SCANCODE_NUM][2] = 
{
    {ERROR, ERROR},   // Error
    {ESC, ESC},      // Esc
    {'1', '!'}, 
    {'2', '@'},
    {'3', '#'}, 
    {'4', '$'},
    {'5', '%'}, 
    {'6', '^'},
    {'7', '&'}, 
    {'8', '*'},
    {'9', '('}, 
    {'0', ')'},
    {'-', '_'}, 
    {'=', '+'},
    {BACKSPACE, BACKSPACE},  
    {' ', ' '},      // Tab
    {'q', 'Q'}, 
    {'w', 'W'},
    {'e', 'E'}, 
    {'r', 'R'},
    {'t', 'T'}, 
    {'y', 'Y'},
    {'u', 'U'}, 
    {'i', 'I'},
    {'o', 'O'}, 
    {'p', 'P'},
    {'[', '{'}, 
    {']', '}'},
    {ENTER, ENTER},  
    {CTRL, CTRL},      // Control
    {'a', 'A'}, 
    {'s', 'S'},
    {'d', 'D'}, 
    {'f', 'F'},
    {'g', 'G'}, 
    {'h', 'H'},
    {'j', 'J'}, 
    {'k', 'K'},
    {'l', 'L'}, 
    {';', ':'},
    {'\'', '"'}, 
    {'`', '~'},
    {LSHIFT, LSHIFT},     // Left Shift
    {'\\', '|'},   
    {'z', 'Z'}, 
    {'x', 'X'},
    {'c', 'C'}, 
    {'v', 'V'},
    {'b', 'B'}, 
    {'n', 'N'},
    {'m', 'M'}, 
    {',', '<'},
    {'.', '>'}, 
    {'/', '?'},
    {RSHIFT, RSHIFT},     // Right Shift
    {PRTSC, PRTSC},     // PrtSc
    {ALT, ALT},     // Alt
    {' ', ' '},     // Space
};


/*
 * keyboard_init
 *  DESCRIPTION: Enables IRQ1 in the PIC so that the keyboard is initialized
 *  INPUTS: None
 *  OUTPUTS: None
 *  RETURN VALUE: None
 *  SIDE EFFECTS: Accepts keyboard interrupts since IRQ1 is now enabled in the PIC
 */
void keyboard_init(void) {
  enable_irq(KEYBOARD_IRQ); //enable irq1

  // set flags 
  flag_caps = 0;
  flag_shift = 0;
  ctrl_flag = 0;
  ctr = 0;
  flag_clear = 0;
  tab_flag = 0;
  alt_press = 0;
}

/*
 * keyboard_irq_handler
 *  DESCRIPTION: Takes input from the keyboard during a keyboard interrupt and outputs it to the screen
 *  INPUTS: None
 *  OUTPUTS: None
 *  RETURN VALUE: None
 *  SIDE EFFECTS: Prints the pressed character to the screen
 */
void keyboard_irq_handler(void) {
  send_eoi(KEYBOARD_IRQ);// send eoi to pic
  uint8_t scancode = inb(KEYBOARD_PORT_DATA) & FIRST_8_BITS; // set scancode
  keyboard_buff[KB_BUF_SIZE - 1] = '\n';
  flag_enter = 0;

  /* checks to see if the scancode value is greater than the number of available scancodes */
  if (scancode > SCANCODE_NUM && 
      scancode != R_SHIFT_R && scancode != L_SHIFT_R &&
      scancode != CAPSLOCK &&
      scancode != LCTRL_SHIFT_R && scancode != TAB_RELEASE && scancode != ALT_R
      && scancode != F1 && scancode != F2 && scancode != F3) {
    return;
  }

  // Things our function should do when enter is pressed
  if (scancode == ENTER) {
    flag_enter = 1; // set flag to 1 if scancode is enter
    
    int i;
    for (i = 0; i < ctr; i++) { // traverse through keyboard
      tempkeyboard_buff[i] = keyboard_buff[i]; // Setting keyboard buffer data to a temp buffer before it's cleared
      keyboard_buff[i] = 0;
    }
     tempkeyboard_buff[ctr] = '\n';
    putc('\n'); // newline command
  }

  if (scancode > 1 && ctr >= 0) {

    /* tab functionality */
    if (scancode == TAB_PRESS) {
      tab_flag = 1; // Setting tab flag
    }

    if (scancode == TAB_RELEASE) {
      tab_flag = 0; // Setting tab flag
    }

    // sets the alt_press flag for the alt keys
    if (scancode == ALT_LEFT || scancode == ALT_RIGHT) {
      alt_press = 1;
      return;
    } else if (scancode == ALT_R) {
      alt_press = 0;
      return;
    }

    // if alt+f1 is pressed, switch to terminal 0
    if (scancode == F1) {
      if (alt_press == 1) {
        terminal_switch(0);
        return;
      }
    }

    // if alt+f2 is pressed, switch to terminal 1
    if (scancode == F2) {
      if (alt_press == 1) {
        terminal_switch(1);
        return;
      }
    }

    // if alt+f3 is pressed, switch to terminal 2
    if (scancode == F3) {
      if (alt_press == 1) {
        terminal_switch(2);
        return;
      }
    }

    // if alt isn't pressed, then just return
    if ((scancode == F1 || scancode == F2 || scancode == F3) && alt_press != 1) {
      return;
    }

    // Case for when tab is pressed.
    // Putc a space for the next four chars on the buffer - 4 spaces for tab
    if (tab_flag == 1) {
      if (ctr != KB_BUF_SIZE - 1) {
        keyboard_buff[ctr] = scan_codes[scancode][0];
        putc(keyboard_buff[ctr]);
        ctr++;
      }
      
      if (ctr != KB_BUF_SIZE - 1) {
        keyboard_buff[ctr] = scan_codes[scancode][0];
        putc(keyboard_buff[ctr]);
        ctr++;
      }

      if (ctr != KB_BUF_SIZE - 1) {
        keyboard_buff[ctr] = scan_codes[scancode][0];
        putc(keyboard_buff[ctr]);
        ctr++;
      }

      if (ctr != KB_BUF_SIZE - 1) {
        keyboard_buff[ctr] = scan_codes[scancode][0];
        putc(keyboard_buff[ctr]);
        ctr++;
      }
    }

    /*Left and right control key*/
    if (scancode == LCTRL_SHIFT_P){
      ctrl_flag = 1;
      return;
    }

    if (scancode == LCTRL_SHIFT_R){
      ctrl_flag = 0;
      return;
    }

     if (scancode == RCTRL_SHIFT_P){
      ctrl_flag = 1;
      return;
    }

    if (scancode == RCTRL_SHIFT_R){
      ctrl_flag = 0;
      return;
    }

    /* if the counter is less than 0, it returns from the function */
    if (ctr < 0) {
      return;
    }
 
    /* backspace rules when in between the buffer */
    if (ctr >= 0 && ctr < KB_BUF_SIZE - 1){
      if (scancode == BACKSPACE) {
        if (ctr > 0) {
          keyboard_buff[ctr-1] = '\0';
          putc(keyboard_buff[ctr-1]);
          ctr--;
        }
      }
    }

    /* backspace and clearing rules when at the end of the buffer */
    if (ctr == KB_BUF_SIZE - 1){
      if (ctrl_flag && ((scan_codes[scancode][1] == 'L') || (scan_codes[scancode][1] == 'l'))) {
          ctr = 0;
          clear();
      } else if (scancode == BACKSPACE) {
        keyboard_buff[ctr-1] = '\0';
          putc(keyboard_buff[ctr-1]);
          ctr--;
      } else {
          return;
      }
    }

    /* xor's the caps lock flag when we press the capslock flag */
    if (scancode == CAPSLOCK){
      flag_caps = ~flag_caps;
      return;
    }

    /* sets the flags for the left and right shift buttons */
    if (scancode == L_SHIFT_P || scancode == R_SHIFT_P){
      flag_shift = 1;
      return;
    }

    if (scancode == L_SHIFT_R || scancode == R_SHIFT_R){
      flag_shift = 0;
      return;
    }

    /* prints the correct letters, numbers, and special characters based on shift or caps lock  */
    if (flag_caps && (flag_shift == 0) && scancode != BACKSPACE && scancode != ENTER){
      // prints the capital letters
      if (((scancode >= Z_BUTTON) && (scancode <= M_BUTTON)) || 
        ((scancode >= A_BUTTON) && (scancode <= L_BUTTON)) ||
        ((scancode >= Q_BUTTON) && (scancode <= P_BUTTON))){
          keyboard_buff[ctr] = scan_codes[scancode][1];
          putc(keyboard_buff[ctr]);
          ctr++;
        } else {  // prints the non-capital letters
        keyboard_buff[ctr] = scan_codes[scancode][0];
        putc(keyboard_buff[ctr]);
        ctr++;
        }
    } else if (flag_shift && (flag_caps == 0) && scancode != BACKSPACE && scancode != ENTER){ // prints the capital letters
      keyboard_buff[ctr] = scan_codes[scancode][1];
      putc(keyboard_buff[ctr]);
      ctr++;
    } else if (flag_caps && flag_shift && scancode != BACKSPACE &&
        scancode != ENTER) {  // prints the non-capital letters
      if (((scancode >= Z_BUTTON) && (scancode <= M_BUTTON)) ||
          ((scancode >= A_BUTTON) && (scancode <= L_BUTTON)) ||
          ((scancode >= Q_BUTTON) && (scancode <= P_BUTTON))){
            keyboard_buff[ctr] = scan_codes[scancode][0];
            putc(keyboard_buff[ctr]);
            ctr++;
          }
      else {  // prints remaining characters
        keyboard_buff[ctr] = scan_codes[scancode][1];
        putc(keyboard_buff[ctr]);
        ctr++;
      }
    }
    else if (scancode != BACKSPACE && flag_enter == 0 && scancode != ENTER) {   // prints special characters
      keyboard_buff[ctr] = scan_codes[scancode][0];
      putc(keyboard_buff[ctr]);
      ctr++;
    }
    
    /* clears the screen when control+l or control+L */
    if (ctrl_flag && ((scan_codes[scancode][1] == 'L') ||
                      (scan_codes[scancode][1] == 'l'))) {
      ctr = 0;
      clear();
      printf("391OS> ");
      flag_clear = 1;
    } 
    if (ctrl_flag && ((scan_codes[scancode][1] == 'L') ||
                      (scan_codes[scancode][1] == 'l'))) {
      ctr = 0;
      clear();
      printf("391OS> ");
      flag_clear = 1;
    }

    // Implementing ctrl+c functionality for halting a currently exectuting process
    // Doing a backspace to delete the c which it puts out
    // changing frequency back to 2Hz
    if (ctrl_flag && ((scan_codes[scancode][1] == 'C') ||
                      (scan_codes[scancode][1] == 'c'))){
                        keyboard_buff[ctr-1] = '\0';
                        putc(keyboard_buff[ctr-1]);
                        ctr--;
                        printf("Program Terminating upon Interrupt\n");
                        rtc_rate_change(2); // magic number - sets the rtc rate back to 2
                        halt(0);
                      }
    
  }
  else
    return;   /* NULL Check*/
}

