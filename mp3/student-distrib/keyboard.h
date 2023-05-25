#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"
#include "i8259.h"
#include "lib.h"
#include "scancodes.h"

#include "terminal.h"

#define KEYBOARD_IRQ  1
#define KEYBOARD_PORT_DATA  0x60

#define FIRST_8_BITS  0xFF  // takes just the first 8 bits of the input data from the keyboard
#define KB_BUF_SIZE   128

/* range all letters on the keyboard */
#define Q_BUTTON      0x10
#define P_BUTTON      0x19
#define A_BUTTON      0x1E
#define L_BUTTON      0x26
#define Z_BUTTON      0x2C
#define M_BUTTON      0x32
#define C_BUTTON      0x2E

/* range of all numbers on the keyboard */
#define ONE_BUTTON    0x02
#define ZERO_BUTTON   0x0B

// #define KB_BUF_SIZE   128

/* Setting required keys' scancodes */
#define ENTER             0x1C
#define R_SHIFT_P         0x36 
#define L_SHIFT_P         0x2A
#define R_SHIFT_R         0xB6 
#define L_SHIFT_R         0xAA
#define CAPSLOCK          0x3A
#define LCTRL_SHIFT_P     0x1D
#define RCTRL_SHIFT_P     0x1D
#define LCTRL_SHIFT_R     0x9D 
#define RCTRL_SHIFT_R     0x9D
#define TAB_PRESS         0x0F
#define TAB_RELEASE       0x8F

#define ALT_LEFT          0x38
#define ALT_RIGHT         0x38

#define ALT_R             0xB8

#define F1                0x3B
#define F2                0x3C
#define F3                0x3D

int entertemp;
int tab_flag;
int flag_clear;
int flag_caps;
int flag_shift;
int ctrl_flag;
int ctr;
int flag_enter;
int alt_press;

/* Enables IRQ1 in the PIC so that the keyboard is initialized */
void keyboard_init(void);

/* Takes input from the keyboard during a keyboard interrupt and outputs it to the screen */
void keyboard_irq_handler(void);

/* Global variables initializations- flags and counters we wish to set to zero just the once*/
char keyboard_buff[KB_BUF_SIZE];
char tempkeyboard_buff[KB_BUF_SIZE];


#endif
