/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask = MASKING; /* IRQs 0-7  */
uint8_t slave_mask = MASKING;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */
/*
 * i8259_init
 *  DESCRIPTION: This function does the initialization sequence for the i8259 PIC then enables the slave
 *  INPUTS: None
 *  OUTPUTS: None
 *  RETURN VALUE: None
 *  SIDE EFFECTS: Initializes the i8259 PIC
 */
void i8259_init(void) {
  /* masks the data */
  outb(MASKING, MASTER_8259_DATA);
  outb(MASKING, SLAVE_8259_DATA);
  
  /* initialization command */
  outb(ICW1, MASTER_8259_PORT);
  outb(ICW1, SLAVE_8259_PORT);

  /* gives the PIC its vector offset */
  outb(ICW2_MASTER, MASTER_8259_DATA);
  outb(ICW2_SLAVE, SLAVE_8259_DATA);

  /* tells master about the slave and gives the slave its identity - IRQ2*/
  outb(ICW3_MASTER, MASTER_8259_DATA);
  outb(ICW3_SLAVE, SLAVE_8259_DATA);

  /* the select mode for the PICs */
  outb(ICW4, MASTER_8259_DATA);
  outb(ICW4, SLAVE_8259_DATA);
  
  /* gives the original data back */
  outb(master_mask, MASTER_8259_DATA);
  outb(slave_mask, SLAVE_8259_DATA);

  /* enables the slave */
  enable_irq(SLAVE_IRQ);
}

/* Enable (unmask) the specified IRQ */
/*
 * enable_irq
 *  DESCRIPTION: This function enables the specified IRQ
 *  INPUTS: irq_num - the specified IRQ that ranges from 0 to 15
 *  OUTPUTS: None
 *  RETURN VALUE: None
 *  SIDE EFFECTS: Allows the PIC to enable/accept the IRQ input
 */
void enable_irq(uint32_t irq_num) {
  uint32_t irq_shift;

  /* checks to see if IRQ number is 16 or greater since there's only 0 to 15 IRQs */
  if (irq_num >= IRQ_PORT_NUMS) {
    return;
  }

  if (irq_num < PIC_PORT_NUMS) {    /* for IRQ numbers 0 to 7 which is on the master PIC */
    master_mask = master_mask & ~(1 << irq_num);
    outb(master_mask, MASTER_8259_DATA);
    return;
}
  else {    /* for IRQ numbers 8 to 15 which is on the slave PIC */
    irq_shift = irq_num - PIC_PORT_NUMS;
    slave_mask = slave_mask & ~(1 << irq_shift);
    outb(slave_mask, SLAVE_8259_DATA);
    return;
}

}

/* Disable (mask) the specified IRQ */
/*
 * disable_irq
 *  DESCRIPTION: This function disables the specified IRQ
 *  INPUTS: irq_num - the specified IRQ that ranges from 0 to 15
 *  OUTPUTS: None
 *  RETURN VALUE: None
 *  SIDE EFFECTS: Disables/masks the PIC from allowing interrupts
 */
void disable_irq(uint32_t irq_num) {
  uint32_t irq_shift;

  /* checks to see if IRQ number is 16 or greater since there's only 0 to 15 IRQs */
  if (irq_num >= IRQ_PORT_NUMS) {
    return;
  }

  if (irq_num < PIC_PORT_NUMS) {    /* for IRQ numbers 0 to 7 which is on the master PIC */
    master_mask |= (1 << irq_num);
    outb(master_mask, MASTER_8259_DATA);
  } else {    /* for IRQ numbers 8 to 15 which is on the slave PIC */
    irq_shift = irq_num - PIC_PORT_NUMS;
    slave_mask |= (1 << irq_shift);
    outb(slave_mask, SLAVE_8259_DATA);
  }

}

/* Send end-of-interrupt signal for the specified IRQ */
/*
 * send_eoi
 *  DESCRIPTION: This function sends the end-of-interrupt signal for the specified IRQ
 *  INPUTS: irq_num - the specified IRQ that ranges from 0 to 15
 *  OUTPUTS: None
 *  RETURN VALUE: None
 *  SIDE EFFECTS: Interrupt has finished executing
 */
void send_eoi(uint32_t irq_num) {
  uint32_t irq_shift;

  /* checks to see if IRQ number is 16 or greater since there's only 0 to 15 IRQs */
  if (irq_num >= IRQ_PORT_NUMS) {
    return;
  }
  
  /* sends EOI signal to slave or master based on irq_num */
  if (irq_num >= PIC_PORT_NUMS) {
    /* sends EOI to the slave and the master through IRQ2 */
    irq_shift = irq_num - PIC_PORT_NUMS;
    outb(EOI | irq_shift, SLAVE_8259_PORT);
    outb(EOI | SLAVE_IRQ, MASTER_8259_PORT);
  } else {
    /* sends EOI to the master */
    outb(EOI | irq_num, MASTER_8259_PORT);
  }

}
