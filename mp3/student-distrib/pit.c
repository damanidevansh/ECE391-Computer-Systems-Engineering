#include "pit.h"

/*
 * init_pit
 *  DESCRIPTION: Initializes the PIT
 *  INPUTS: None
 *  OUTPUTS: None
 *  RETURN VALUE: None
 *  SIDE EFFECTS: Allows the PIT interrupts
 */
void init_pit() {
  outb(PIT_SQUARE_WAVE, PIT_CMD_REG);         // sets the command byte
  outb(DIVISOR && MASK, PIT_DATA);            // sets the low byte of the divisor
  outb(DIVISOR >> FREQ_BIT_SHIFT, PIT_DATA);  // sets the high byte of divisor

  enable_irq(PIT_IRQ); // enabling the pit interrupt
}

/*
 * pit_irq_handler
 *  DESCRIPTION: Handles the PIT interrupt and then does a context switch
 *  INPUTS: None
 *  OUTPUTS: None
 *  RETURN VALUE: None
 *  SIDE EFFECTS: Switches to another terminal
 */
void pit_irq_handler() {
  send_eoi(PIT_IRQ); // sending end of interrupt
}

