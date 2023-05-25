#ifndef IDT_H
#define IDT_H
#ifndef ASM

#include "lib.h"
#include "interrupts.h"
#include "system_calls.h"
#include "x86_desc.h"

#define EXCEPTION_TOTAL     20   // Number of exceptions we are using
#define SYSTEM_CALL_VECTOR  0x80 // system call vector
#define KEYBOARD_VECTOR     0x21 // keyboard IDT vector
#define RTC_VECTOR          0x28 // RTC IDT vector
#define PIT_VECTOR          0x20 // PIT IDT vector

/* sets the registers that are passed in to the exception handler */
struct x86_regs {
   uint32_t edi;
   uint32_t esi;
   uint32_t ebp;
   uint32_t esp;
   uint32_t ebx;
   uint32_t edx;
   uint32_t ecx;
   uint32_t eax;
} __attribute__ (( packed ));

/* This function initializes the IDT and its properties */
void idt_init(void);

/* Exception declarations */

// Handles the division error by printing the message and running forever
void division_error();

// Handles the debug error by printing the message and running forever
void debug();

// Handles the nmi error by printing the message and running forever
void nmi();

// Handles the breakpoint error by printing the message and running forever
void breakpoint();

// Handles the overflow error by printing the message and running forever
void overflow();

// Handles the bound error by printing the message and running forever
void bound();

// Handles the opcode error by printing the message and running forever
void opcode();

//  Handles the device not available error by printing the message and running forever
void device_not_available();

// Handles the double fault error by printing the message and running forever 
void double_fault();

// Handles the segment overrun error by printing the message and running forever
void segment_overrun();

// Handles the invalid tss error by printing the message and running forever
void invalid_tss();

// Handles the segment not present error by printing the message and running forever
void segment_not_present();

// Handles the stack fault error by printing the message and running forever
void stack_fault();

// Handles the general protection fault error by printing the message and running forever
void general_protection_fault();

// Handles the page fault error by printing the message and running forever
void page_fault();

// Handles the reserved exception (exception 15)
void not_used(); 

// Handles the floating point error by printing the message and running forever
void floating_point();

// Handles the alignment check error by printing the message and running forever
void alignment_check();

// Handles the machine check error by printing the message and running forever
void machine_check();

// Handles the simd error by printing the message and running forever
void simd();


#endif
#endif
