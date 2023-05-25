#ifndef PIT_H
#define PIT_H

#include "i8259.h"
#include "types.h"
#include "lib.h"
#include "terminal.h"

/* sets the definitions used in the .c code */
#define PIT_SQUARE_WAVE 0x36
#define PIT_CMD_REG     0x43   // command register port
#define DIVISOR         11932  // frequency divisor
#define MASK            0xFF
#define PIT_DATA        0x40  // channel 0 data register port
#define FREQ_BIT_SHIFT  8     // number of bits to shift frequency by
#define PIT_IRQ         0     // IRQ for pit

/* Initializes the PIT */
void init_pit();

/* Handles the PIT interrupt and then does a context switch */
void pit_irq_handler(); 

#endif
