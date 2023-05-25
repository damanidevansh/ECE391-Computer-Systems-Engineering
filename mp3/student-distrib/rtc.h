#ifndef RTC_H
#define RTC_H

#include "lib.h"
#include "types.h"
#include "i8259.h"

#define RTC_IRQ     8     // IRQ for rtc
#define DISABLE_NMI 0x70
#define RTC_DATA    0x71
#define BIT_SIX     0x40

/* frequencies and rates */
#define MAX_FREQUENCY 1024  
#define MIN_FREQUENCY 2
#define MIN_RATE      3
#define MAX_RATE      15

/* The RTC registers */
#define REGISTER_A  0x8A
#define REGISTER_B  0x8B
#define REGISTER_C  0x0C


/* This function initializes the RTC interrupt and sets the rate */
void rtc_init(void);

/* This function reads data frome register C to handle the RTC interrupt */
void rtc_irq_handler(void);

/* This function opens the RTC and sets the frequency to 2 Hz */
int32_t rtc_open(const uint8_t* filename);

/* This function closes the RTC and sets the frequency to 2 Hz */
int32_t rtc_close(int32_t fd);

/* This function reads the items in the RTC after the interrupt */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);

/* This function determines the RTC interrupt rate */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);

/* This function changes the RTC rate through the input frequency */
void rtc_rate_change(uint32_t frequency);

/* This function gives a rate for the provided frequency */
int32_t find_rate(uint32_t frequency);


#endif
