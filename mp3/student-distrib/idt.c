#include "idt.h"
#include "system_calls.h"

/*
 * idt_init
 *  DESCRIPTION: This function initializes the IDT and its properties
 *  INPUTS: None
 *  OUTPUTS: None
 *  RETURN VALUE: None
 *  SIDE EFFECTS: Sets the values for the IDT for the exceptions, device interrupts, and the system call
 */
void idt_init(void){
  int i;
  for(i = 0; i < NUM_VEC; i++) { // Loop through the number of vectors in the IDT
    
    if (i < EXCEPTION_TOTAL){ // 15 is reserved in the vector table so we are unable to use it
      idt[i].present = 1; // command is present in the table 
    } else {
      idt[i].present = 0;
    }

    idt[i].seg_selector = KERNEL_CS;
    idt[i].dpl = 0;
    idt[i].reserved0 = 0;
    idt[i].reserved1 = 1; 
    idt[i].reserved2 = 1;
    idt[i].reserved3 = 1; // need to set to 0 for interrupts
    idt[i].reserved4 = 0;
    idt[i].size = 1;
  }    

  /* sets the exceptions for exceptions 0 to 19 not including 15 */
  SET_IDT_ENTRY(idt[0], division_error);
  SET_IDT_ENTRY(idt[1], debug);
  SET_IDT_ENTRY(idt[2], nmi);
  SET_IDT_ENTRY(idt[3], breakpoint);
  SET_IDT_ENTRY(idt[4], overflow);
  SET_IDT_ENTRY(idt[5], bound);
  SET_IDT_ENTRY(idt[6], opcode);
  SET_IDT_ENTRY(idt[7], device_not_available);
  SET_IDT_ENTRY(idt[8], double_fault);
  SET_IDT_ENTRY(idt[9], segment_overrun);
  SET_IDT_ENTRY(idt[10], invalid_tss);
  SET_IDT_ENTRY(idt[11], segment_not_present);
  SET_IDT_ENTRY(idt[12], stack_fault);
  SET_IDT_ENTRY(idt[13], general_protection_fault);
  SET_IDT_ENTRY(idt[14], page_fault);
  SET_IDT_ENTRY(idt[15], not_used);   /* resereved exception, should not be used */
  SET_IDT_ENTRY(idt[16], floating_point);
  SET_IDT_ENTRY(idt[17], alignment_check);
  SET_IDT_ENTRY(idt[18], machine_check);
  SET_IDT_ENTRY(idt[19], simd);

  /* sets the system call */
  SET_IDT_ENTRY(idt[SYSTEM_CALL_VECTOR], system_call_handler);
  idt[SYSTEM_CALL_VECTOR].present = 0x1;
  idt[SYSTEM_CALL_VECTOR].dpl = 0x3;  // 0x3 magic number - sets the dpl value to 3, needed for sys calls

  /* sets the device interrupts */
  idt[KEYBOARD_VECTOR].present = 0x1;
  idt[KEYBOARD_VECTOR].reserved3 = 0;
  idt[RTC_VECTOR].present = 0x1;
  idt[RTC_VECTOR].reserved3 = 0;
  idt[PIT_VECTOR].present = 1;
  idt[PIT_VECTOR].reserved3 = 0;
  SET_IDT_ENTRY(idt[KEYBOARD_VECTOR], keyboard_interrupt);
  SET_IDT_ENTRY(idt[RTC_VECTOR], RTC_interrupt);
  SET_IDT_ENTRY(idt[PIT_VECTOR], PIT_interrupt);
}

/*
 * division_error
 *
 *  DESCRIPTION: Handles the division error by printing the message and running forever
 *  INPUTS: None
 *  OUTPUTS: Prints a message
 *  RETURN VALUE: None
 *  SIDE EFFECTS: None
 */
void division_error() {
  printf("Divided by 0 error\n");
  halt((uint32_t)256); // 256 magic number - halts the program after the exception occurs
}

/*
 * debug
 *  
 *  INPUTS: None
 *  OUTPUTS: Prints a message
 *  RETURN VALUE: None
 *  SIDE EFFECTS: None
 */
void debug() {
  printf("Debug error\n");
  halt((uint32_t)256);// 256 magic number - halts the program after the exception occurs 
}

/*
 * nmi
 *  DESCRIPTION: Handles the nmi error by printing the message and running forever
 *  INPUTS: None
 *  OUTPUTS: Prints a message
 *  RETURN VALUE: None
 *  SIDE EFFECTS: None
 */
void nmi() {
  printf("Non-maskable Interrupt error\n");
  halt((uint32_t)256);// 256 magic number - halts the program after the exception occurs
}

/*
 * breakpoint
 *  DESCRIPTION: Handles the breakpoint error by printing the message and running forever
 *  INPUTS: None
 *  OUTPUTS: Prints a message
 *  RETURN VALUE: None
 *  SIDE EFFECTS: None
 */
void breakpoint(){
  printf("Breakpoint error\n");
  halt((uint32_t)256);// 256 magic number - halts the program after the exception occurs
}

/*
 * overflow
 *  DESCRIPTION: Handles the overflow error by printing the message and running forever
 *  INPUTS: None
 *  OUTPUTS: Prints a message
 *  RETURN VALUE: None
 *  SIDE EFFECTS: None
 */
void overflow() {
  printf("Overflow error\n");
  halt((uint32_t)256);// 256 magic number - halts the program after the exception occurs
}

/*
 * bound
 *  DESCRIPTION: Handles the bound error by printing the message and running forever
 *  INPUTS: None
 *  OUTPUTS: Prints a message
 *  RETURN VALUE: None
 *  SIDE EFFECTS: None
 */
void bound() {
  printf("Bound range exceeded error\n");
  halt((uint32_t)256);// 256 magic number - halts the program after the exception occurs
}

/*
 * opcode
 *  DESCRIPTION: Handles the opcode error by printing the message and running forever
 *  INPUTS: None
 *  OUTPUTS: Prints a message
 *  RETURN VALUE: None
 *  SIDE EFFECTS: None
 */
void opcode() {
  printf("Opcode error\n");
  halt((uint32_t)256);// 256 magic number - halts the program after the exception occurs
}

/*
 * device_not_available
 *  DESCRIPTION: Handles the device not available error by printing the message and running forever
 *  INPUTS: None
 *  OUTPUTS: Prints a message
 *  RETURN VALUE: None
 *  SIDE EFFECTS: None
 */
void device_not_available() {
  printf("Device not available error\n");
  halt((uint32_t)256);// 256 magic number - halts the program after the exception occurs
}

/*
 * double_fault
 *  DESCRIPTION: Handles the double fault error by printing the message and running forever
 *  INPUTS: None
 *  OUTPUTS: Prints a message
 *  RETURN VALUE: None
 *  SIDE EFFECTS: None
 */
void double_fault() {
  printf("Double fault error\n");
  halt((uint32_t)256);// 256 magic number - halts the program after the exception occurs
}

/*
 * segment_overrun
 *  DESCRIPTION: Handles the segment overrun error by printing the message and running forever
 *  INPUTS: None
 *  OUTPUTS: Prints a message
 *  RETURN VALUE: None
 *  SIDE EFFECTS: None
 */
void segment_overrun() {
  printf("Segment overrun error\n");
  halt((uint32_t)256);// 256 magic number - halts the program after the exception occurs
}

/*
 * invalid_tss
 *  DESCRIPTION: Handles the invalid tss error by printing the message and running forever
 *  INPUTS: None
 *  OUTPUTS: Prints a message
 *  RETURN VALUE: None
 *  SIDE EFFECTS: None
 */
void invalid_tss() {
  printf("Invalid TSS error\n");
  halt((uint32_t)256);// 256 magic number - halts the program after the exception occurs
}

/*
 * segment_not_present
 *  DESCRIPTION: Handles the segment not present error by printing the message and running forever
 *  INPUTS: None
 *  OUTPUTS: Prints a message
 *  RETURN VALUE: None
 *  SIDE EFFECTS: None
 */
void segment_not_present() {
  printf("Segment not present error\n");
  halt((uint32_t)256);// 256 magic number - halts the program after the exception occurs
}

/*
 * stack_fault
 *  DESCRIPTION: Handles the stack fault error by printing the message and running forever
 *  INPUTS: None
 *  OUTPUTS: Prints a message
 *  RETURN VALUE: None
 *  SIDE EFFECTS: None
 */
void stack_fault() {
  printf("Stack fault error\n");
  halt((uint32_t)256);// 256 magic number - halts the program after the exception occurs
}

/*
 * general_protection_fault
 *  DESCRIPTION: Handles the general protection fault error by printing the message and running forever
 *  INPUTS: None
 *  OUTPUTS: Prints a message
 *  RETURN VALUE: None
 *  SIDE EFFECTS: None
 */
void general_protection_fault() {
  printf("General protection fault error\n");
  halt((uint32_t)256);// 256 magic number - halts the program after the exception occurs
}

/*
 * page_fault
 *
 *  DESCRIPTION: Handles the page fault error by printing the message and running forever
 *  INPUTS: None
 *  OUTPUTS: Prints a message
 *  RETURN VALUE: None
 *  SIDE EFFECTS: None
 */
void page_fault() {
  printf("Page fault error\n");
  halt((uint32_t)256);// 256 magic number - halts the program after the exception occurs
}

/*
 * not_used
 *
 *  DESCRIPTION: Handles the reserved exception (exception 15)
 *  INPUTS: None
 *  OUTPUTS: Prints a message
 *  RETURN VALUE: None
 *  SIDE EFFECTS: None
 */
void not_used() {
  printf("This should not be raised\n");
  halt((uint32_t)256);// 256 magic number - halts the program after the exception occurs
}

/*
 * floating_point
 *  DESCRIPTION: Handles the floating point error by printing the message and running forever
 *  INPUTS: None
 *  OUTPUTS: Prints a message
 *  RETURN VALUE: None
 *  SIDE EFFECTS: None
 */
void floating_point() {
  printf("Floating point error\n");
  halt((uint32_t)256);// 256 magic number - halts the program after the exception occurs
}

/*
 * alignment_check
 *  DESCRIPTION: Handles the alignment check error by printing the message and running forever
 *  INPUTS: None
 *  OUTPUTS: Prints a message
 *  RETURN VALUE: None
 *  SIDE EFFECTS: None
 */
void alignment_check() {
  printf("Alignment check error\n");
  halt((uint32_t)256);// 256 magic number - halts the program after the exception occurs
}

/*
 * machine_check
 *  DESCRIPTION: Handles the machine check error by printing the message and running forever
 *  INPUTS: None
 *  OUTPUTS: Prints a message
 *  RETURN VALUE: None
 *  SIDE EFFECTS: None
 */
void machine_check() {
  printf("Machine check error\n");
  halt((uint32_t)256);// 256 magic number - halts the program after the exception occurs
}

/*
 * simd
 *  DESCRIPTION: Handles the simd error by printing the message and running forever
 *  INPUTS: None
 *  OUTPUTS: Prints a message
 *  RETURN VALUE: None
 *  SIDE EFFECTS: None
 */
void simd() {
  printf("SIMD error\n");
  halt((uint32_t)256);// 256 magic number - halts the program after the exception occurs
}
