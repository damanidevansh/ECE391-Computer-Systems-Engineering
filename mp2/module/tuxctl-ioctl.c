/* tuxctl-ioctl.c
 *
 * Driver (skeleton) for the mp2 tuxcontrollers for ECE391 at UIUC.
 *
 * Mark Murphy 2006
 * Andrew Ofisher 2007
 * Steve Lumetta 12-13 Sep 2009
 * Puskar Naha 2013
 */

#include <asm/current.h>
#include <asm/uaccess.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/kdev_t.h>
#include <linux/tty.h>
#include <linux/spinlock.h>

#include "tuxctl-ld.h"
#include "tuxctl-ioctl.h"
#include "mtcp.h"

#define debug(str, ...) \
	printk(KERN_DEBUG "%s: " str, __FUNCTION__, ## __VA_ARGS__)


// Global variable declarations, which are used across functions
// MAGIC NUMBER- 0xF or 4'b1111 is used as a mask to obtain the 4 LSBs of a certain number.
unsigned packet_a, packet_b, packet_c;
char set_leds[6];
char buttons = 0;
int ack;
int mask = 0xF;

/************************ Protocol Implementation *************************/

/* tuxctl_handle_packet()
 * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in 
 * tuxctl-ld.c. It calls this function, so all warnings there apply 
 * here as well.
 */
void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet)
{
    unsigned a, b, c;

    a = packet[0]; /* Avoid printk() sign extending the 8-bit */
    b = packet[1]; /* values when printing them. */
    c = packet[2];

	// Assigning each byte of the packet to a different variable. This makes it easier to use.
	packet_a = a;
	packet_b = b;
	packet_c = c;

	// Switch case of interrupts depending on packet a, or the OPCODE.
	switch(a) {
		case MTCP_ERROR:		// Corresponding to an ERROR interrupt. Here, we just want to break out of the function.
			break;
		case MTCP_RESET:		// Corresponding to when RESET is pressed. This calls INIT again, and sets LED values to the last saved ones
			tux_init(tty);
			tuxctl_ldisc_put(tty, set_leds, 6);
			break;
		case MTCP_BIOC_EVENT:	// Corresponds to when a button is pressed. It sets packet_b and packet_c variables which I can use later on.
			packet_b = b;
			packet_c = c;
			break;
		case MTCP_ACK:		// Corresponds to the acknowledge flag- which always sets ack to 0 after a function has finished executing
			ack = 0;
			break;
		default:
			break;
	}
    /*printk("packet : %x %x %x\n", a, b, c); */
}

/******** IMPORTANT NOTE: READ THIS BEFORE IMPLEMENTING THE IOCTLS ************
 *                                                                            *
 * The ioctls should not spend any time waiting for responses to the commands *
 * they send to the controller. The data is sent over the serial line at      *
 * 9600 BAUD. At this rate, a byte takes approximately 1 millisecond to       *
 * transmit; this means that there will be about 9 milliseconds between       *
 * the time you request that the low-level serial driver send the             *
 * 6-byte SET_LEDS packet and the time the 3-byte ACK packet finishes         *
 * arriving. This is far too long a time for a system call to take. The       *
 * ioctls should return immediately with success if their parameters are      *
 * valid.                                                                     *
 *                                                                            *
 ******************************************************************************/
int
tuxctl_ioctl (struct tty_struct* tty, struct file* file, 
	      unsigned cmd, unsigned long arg)
{
    switch (cmd) {
	case TUX_INIT:
		return tux_init(tty); // Calling tux_init function with required arguments in the TUX_INIT case
	case TUX_BUTTONS:
		return tux_buttons(tty, arg); // Calling tux_buttons function with required arguments in the TUX_BUTTONS case
	case TUX_SET_LED:
		return tux_set_leds(tty, arg);  // Calling tux_set_leds function with required arguments in the TUX_LED_SET case
	case TUX_LED_ACK:
	case TUX_LED_REQUEST:
	case TUX_READ_LED:
	default:
	    return -EINVAL;
    }
}

/*
 * tux_init
 *   DESCRIPTION: this initializes the TUX controller to its 'init' state, where the LED values are cleared and the ack flag == 0
 *   INPUTS: the tty struct.
 *   OUTPUTS: outputs a cleared LEDs to the TUX LEDs.
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: Sets ack flag to 0, initializes packet_b and packet_c variables to 0xFF.
 *   MAGIC NUMBER- 0xFF is 8'b11111111- or the '0' version of an active low variable.
 */
int tux_init(struct tty_struct* tty) {
	// Initializing and setting variables to required values
	char init[8];
	ack = 0;
	packet_b = 0xFF;
	packet_c = 0xFF;

	// Setting the init array to required values to output to the TUX, based off documentation
	init[0] = MTCP_BIOC_ON; // OPCODES
	init[1] = MTCP_LED_USR;
	init[2] = MTCP_LED_SET;
	init[3] = 0xF;	/* MAGIC NUMBER- Tells us which LEDs we want to manipulate (all 4 in our case), so we want 1111, or 0xF */
	init[4] = 0;	/* Rightmost LED*/
	init[5] = 0;
	init[6] = 0;
	init[7] = 0;	/* Leftmost LED */

	tuxctl_ldisc_put(tty, init, 8); // Function call to put init onto tty
									// MAGIC NUMBER- 8 signifies 8 bytes (the size of the init char) to be out onto the tty struct
	return 0;
}

// Initializing char for displaying hex values. Was giving me a warning when I did it inside the fucntion.
char hexvalues[16];


/*
 * tux_set_leds
 *   DESCRIPTION: This function sets the LEDs to the values we want.
 *   INPUTS: the tty struct and the argument of the 32-bit integer to tell us which exact LEDs to change, and to what
 *   OUTPUTS: outputs a certain display to the TUX LEDs.
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: Sets ack flag to 1
 */
int tux_set_leds(struct tty_struct* tty, unsigned long arg) {
	// ack flag handling- if flag is 1, then don't call the function and ret instantly 
	if (ack == 1) {
		return 0;
	}
	
	// Initializing local variables used in this function
	int led1, led2, led3, led4;
	int dec1, dec2, dec3, dec4;
	unsigned long temp1, temp2, temp3, temp4;

	// Setting ack flag to 1, to indicate the function is being used.
	ack = 1;

	// Setting up set_leds buffer which will be then outputted by the ioctl call.
	set_leds[0] = MTCP_LED_SET; // OPCODE
	set_leds[1] = 0xF; /* MAGIC NUMBER- Tells us which LEDs we want to manipulate (all 4 in our case), so we want 1111, or 0xF */
	set_leds[2] = 0;
	set_leds[3] = 0;
	set_leds[4] = 0;
	set_leds[5] = 0;
	
	// MAGIC NUMBERS USED BELOW:
	// I can't write out the explanation of each of the 16 digits but what is happening here is
	// I am outputting the required bits I need to turn off and on to display the required hex digit
	// based off the index (the comment next to each index explains which digit I am wishing to output).
	// These were calculated based off the format given in mtcp.h
	hexvalues[0] = 0xE7; // 0
	hexvalues[1] = 0x06; // 1
	hexvalues[2] = 0xCB; // 2
	hexvalues[3] = 0x8F; // 3
	hexvalues[4] = 0x2E; // 4
	hexvalues[5] = 0xAD; // 5
	hexvalues[6] = 0xED; // 6
	hexvalues[7] = 0x86; // 7
	hexvalues[8] = 0xEF; // 8
	hexvalues[9] = 0xAF; // 9
	hexvalues[10] = 0xEE; // A
	hexvalues[11] = 0x6D; // B
	hexvalues[12] = 0xE1; // C
	hexvalues[13] = 0x4F; // D
	hexvalues[14] = 0xE9; // E
	hexvalues[15] = 0xE8; // F

	// Performing bit shifting and masking to obtain individual values of arg[19:16] to obtain information
	// on which LED we want to turn on and manipulate.
	// MAGIC numbers used below:
	// 19- 19 corresponds to the 19th bit of the 32-bit argument which tells me whether LED1 is on or not.
	// 18- 18 corresponds to the 18th bit of the 32-bit argument which tells me whether LED1 is on or not.
	// 17- 17 corresponds to the 17th bit of the 32-bit argument which tells me whether LED1 is on or not.
	// 16- 16 corresponds to the 16th bit of the 32-bit argument which tells me whether LED1 is on or not.
	led1 = (arg >> 19) & 0x01;
	led2 = (arg >> 18) & 0x01;
	led3 = (arg >> 17) & 0x01;
	led4 = (arg >> 16) & 0x01;

	// Performing bit shifting and masking to obtain individual values of arg[27:24] to obtain information
	// on which decimal point we want to turn on and manipulate.
	// MAGIC numbers used below:
	// 27- 27 corresponds to the 27th bit of the 32-bit argument which tells me whether decimal point 1 is on or not.
	// 26- 26 corresponds to the 26th bit of the 32-bit argument which tells me whether decimal point 2 is on or not.
	// 25- 25 corresponds to the 25th bit of the 32-bit argument which tells me whether decimal point 3 is on or not.
	// 24- 24 corresponds to the 24th bit of the 32-bit argument which tells me whether decimal point 4 is on or not.
	dec1 = (arg >> 27) & 0x01;
	dec2 = (arg >> 26) & 0x01;
	dec3 = (arg >> 25) & 0x01;
	dec4 = (arg >> 24) & 0x01;

	// If required to change the LED value (when each led(n) variable is high), we bit shift the original argument by
	// the position of where it is located in the 32-bit argument to obtain the output byte as the 4 LSBs, and masked
	// to obtain just the last byte. Then, the main buffer's corresponding element is set to the hexvalue of the bitshifted
	// index to tell the function to output that character.
	if (led1) {
		temp1 = (arg >> 12) & mask; // MAGIC NUMBER- LED1's values are defined in arg[15:12], so we shift by 12 to obtain them as LSBs
		set_leds[5] = hexvalues[temp1];
	}
	if (led2){
		temp2 = (arg >> 8) & mask;  // MAGIC NUMBER- LED2's values are defined in arg[15:12], so we shift by 8 to obtain them as LSBs
		set_leds[4] = hexvalues[temp2];
	}
	if (led3){
		temp3 = (arg >> 4) & mask;  // MAGIC NUMBER- LED3's values are defined in arg[15:12], so we shift by 4 to obtain them as LSBs
		set_leds[3] = hexvalues[temp3];
	}
	if (led4){
		temp4 = arg & mask;
		set_leds[2] = hexvalues[temp4];
	}

	// If required to change the decimal value (when each dec(n) variable is high), we want to change each element of set_led's
	// 4th (including a zero index, so really bit number 5) bit to a 1. To do this, I am ORing the whole thing with 8'b00010000
	// or 0x10.
	// MAGIC NUMBER- This number sets the 4th bit (bit #5) to 1 if it is zero. If it is already 1, it remains 1. It is kind of
	// analogous to a mask, but we are not using AND.
	if(dec4)
		set_leds[2] = set_leds[2] | 0x10;
	if(dec3)
		set_leds[3] = set_leds[3] | 0x10;
	if(dec2)
		set_leds[4] = set_leds[4] | 0x10;
	if(dec1)
		set_leds[5] = set_leds[5] | 0x10;
	
	tuxctl_ldisc_put(tty, set_leds, 6); // ioctl function call to set LEDs by passing set_leds array to tty.
										// MAGIC NUMBER- 6 signifies 6 bytes (the size of the set_leds char) to be out onto the tty struct
	return 0;
}


/*
 * tux_buttons
 *   DESCRIPTION: This function outputs a certain value depending on which button is pressed,
 *                depending on the packets sent to the kernel by TUX.
 *   INPUTS: the tty struct and the argument of the 32-bit integer to tell us which exact LEDs to change, and to what
 *   OUTPUTS: outputs a char 'button' with the encoded data in the required form specified by the TUX_SPEC document. 
 *   RETURN VALUE: 0 if successful, -EINVAL if not.
 *   SIDE EFFECTS: none
 */
extern int tux_buttons(struct tty_struct* tty, unsigned long arg) {
	// Variable declarations
	unsigned char bottom, right, left, down, up;

	// NULL CHECK
	if ((void*) arg == NULL)
		return -EINVAL;

	// We want right, left, down, up, c, b, a, start

	// Shifting and masking to obtain the required bit in its correct position.
	bottom = packet_b & mask; // Isolates C, B, A, START from packet_b

	// packet_c manipulation:
	//  RIght hifting each button bit from its given location in packet_c to make it the LSB,
	// ANDing by 0x01 to obtain just the bit itself
	// and then left shifting it to its required position for button output.
	right = ((packet_c >> 3) & 0x01) << 7; // MAGIC NUMBERS- 3 and 7 are the positions that 'right' is in packet_c and buttons respectively
	left = ((packet_c >> 1) & 0x01) << 6; // MAGIC NUMBERS- 1 and 6 are the positions that 'left' is in packet_c and buttons respectively
	down = ((packet_c >> 2) & 0x01) << 5; // MAGIC NUMBERS- 2 and 5 are the positions that 'down' is in packet_c and buttons respectively
	up = (packet_c & 0x01) << 4; // MAGIC NUMBER- 4 is the positions that 'up' is in buttons

	buttons = right | left | down | up | bottom; // ORing the 4 chars to obtain a final buttons char

	// Copying the buttons to the user space and to the argument pointer.
	// Returns -EINVAL if fails, or if it is bigger than zero.
	if(copy_to_user((void*)arg, &buttons, sizeof(buttons)) > 0)
		return -EINVAL;
 
	return 0;
}
