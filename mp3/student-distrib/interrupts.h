#ifndef INTERRUPTS_H
#define INTERRUPTS_H

/* all of the interrupts as a function */
extern void keyboard_interrupt(void);
extern void RTC_interrupt(void);
extern void PIT_interrupt(void);

#endif
