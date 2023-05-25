#include "rtc.h"
#include "terminal.h"

volatile uint32_t rtc_flag = 1;
int32_t frequencies = 1;
int counter = 0;

volatile unsigned int terminal_count[TERMINAL_NUM];    // count for each terminal - there are 3 terminals 
volatile unsigned int terminal_frequency[TERMINAL_NUM];  // count for each terminal - there are 3 terminals 
volatile int cur_frequency;

/*
 * rtc_init
 *  DESCRIPTION: This function initializes the RTC interrupt and sets the rate
 *  INPUTS: None
 *  OUTPUTS: None
 *  RETURN VALUE: None
 *  SIDE EFFECTS: Accepts RTC interrupts since IRQ8 is now enabled in the PIC
 */
void rtc_init(void) {
  /* initializes the RTC interrupt */
  outb(REGISTER_B, DISABLE_NMI);
  char prev = inb(RTC_DATA);
  outb(REGISTER_B, DISABLE_NMI);
  outb(prev | BIT_SIX, RTC_DATA);
  enable_irq(RTC_IRQ);

  // setting terminal counts and frequencies
  int i;
  for (i = 0; i < TERMINAL_NUM; i++) {
    terminal_count[i] = 0;
    terminal_frequency[i] = 2;  // 2 magic number - sets the frequency to the minimum frequency
  }
  
  cur_frequency = 2; // 2 is the min frquency

  /* changes the rate of the rtc */
  cli();
  outb(REGISTER_A, DISABLE_NMI);
  prev = inb(RTC_DATA);
  outb(REGISTER_A, DISABLE_NMI);  /* resets the index to A to write the rate to A */
  outb((prev & 0xF0) | 5, RTC_DATA); // MAGIC NUMBER- 0xF0 to get first 4 bits
  sti();
}

/*
 * rtc_irq_handler
 *  DESCRIPTION: This function reads data frome register C to handle the RTC interrupt
 *  INPUTS: None
 *  OUTPUTS: None
 *  RETURN VALUE: None
 *  SIDE EFFECTS: Interrupt begins by sending the EOI
 */
void rtc_irq_handler(void) {
  cli();  // mask interurpt

  terminal_count[cur_terminal] +=1;

  /* selects Register C and throws away the contents */
  outb(REGISTER_C, DISABLE_NMI);
  inb(RTC_DATA);
 
  send_eoi(RTC_IRQ);
  sti();
}

/*
 * rtc_open
 *  DESCRIPTION: This function opens the RTC and sets the frequency to 2 Hz
 *  INPUTS: filename - pointer to the file to open
 *  OUTPUTS: None
 *  RETURN VALUE: 0
 *  SIDE EFFECTS: Sets the inital frequency to 2 Hz
 */
int32_t rtc_open(const uint8_t* filename) {
  /* need to reset the frequency to 2 Hz */
  terminal_frequency[cur_terminal] = 2; // set freq of current terminal to 2 - 2 is the MIN frequency
  return 0;
}
 
/*
 * rtc_read
 *  DESCRIPTION: This function reads the items in the RTC after the interrupt
 *  INPUTS: fd - file descriptor number
 *          buf - buffer to read
 *          nbytes - number of bytes to read
 *  OUTPUTS: None
 *  RETURN VALUE: 0
 *  SIDE EFFECTS: None
 */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes) {
  /* sets the rtc flag to 0 then puts it into an infinite while loop until it's set back to 1 */
  rtc_flag = 0;
  terminal_count[cur_terminal] = 0;
  while (terminal_count[cur_terminal] < (1024/cur_frequency)){;} // 1024 is the MAX frequency
  memcpy(buf, (void*)& terminal_frequency[cur_terminal], sizeof(int));
  return 0;
}

/*
 * rtc_close
 *  DESCRIPTION: This function closes the RTC and sets the frequency to 2 Hz
 *  INPUTS: fd - file descriptor number
 *  OUTPUTS: None
 *  RETURN VALUE: 0
 *  SIDE EFFECTS: Sets the frequency back to 2 Hz
 */
int32_t rtc_close(int32_t fd) {
  // set rtc frequency back to 2Hz when we close
  terminal_count[cur_terminal] = 0;
  terminal_frequency[cur_terminal] = 2; // 2 is the MIN frequency
  return 0;
}

/*
 * rtc_write
 *  DESCRIPTION: This function determines the RTC interrupt rate
 *  INPUTS: fd - file descriptor number
 *          buf - buffer to write
 *          nbytes - bytes to write
 *  OUTPUTS: None
 *  RETURN VALUE: -1 for failure, 0 for success
 *  SIDE EFFECTS: Writes the frequency/RTC interrupt rate
 */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes) {
  int temp_freq;
  
  /* ensures a valid input to be written */
  if (nbytes != sizeof(int32_t) || buf == NULL) {
    return -1;
  }

  temp_freq = *((int32_t*)buf); // gets the value of the frequency from the buf

  /* checks to make sure that the frequency is within the valid range of 2 to 1024 Hz */
  if (temp_freq > MAX_FREQUENCY || temp_freq < MIN_FREQUENCY) {
    return -1;
  }

  /* checks to see if the frequency is a power of 2 */
  if ((temp_freq & (temp_freq - 1)) != 0) {
    return -1;
  }

  // set rtc frequency to temp frequency 
  terminal_frequency[cur_terminal] = temp_freq;
  cur_frequency = temp_freq;

  return 0;
}

/*
 * rtc_rate_change
 *  DESCRIPTION: This function changes the RTC rate through the input frequency
 *  INPUTS: frequency - the RTC rate we want to change it to
 *  OUTPUTS: None
 *  RETURN VALUE: None
 *  SIDE EFFECTS: RTC rate changes
 */
void rtc_rate_change(uint32_t frequency) {
  /* produces a rate given the frequency and then checks if the rate within 3 to 15 */
  uint32_t rate = find_rate(frequency);
  if (rate < MIN_RATE || rate > MAX_RATE) {
    return;
  }
  terminal_frequency[cur_terminal] = frequency;
  frequencies = frequency;  // sets the frequency to the globabl variable

}

/*
 * find_rate
 *  DESCRIPTION: This function gives a rate for the provided frequency
 *  INPUTS: frequency - the RTC rate that we input
 *  OUTPUTS: None
 *  RETURN VALUE: a rate based on the input frequency
 *  SIDE EFFECTS: None
 */
int32_t find_rate(uint32_t frequency) {
  /* sets the rate based on the input frequency */
  switch (frequency) {
    case 1024: 
      return 0x06;

    case 512:
      return 0x07;

    case 256:
      return 0x08;

    case 128:
      return 0x09;

    case 64:
      return 0x0A;

    case 32:
      return 0x0B;

    case 16:
      return 0x0C;

    case 8:
      return 0x0D;

    case 4:
      return 0x0E;

    case 2:
      return 0x0F;

    default:  // only accepts power of 2 frequencies up till 1024 hz
      return 0x00;
  }
}
