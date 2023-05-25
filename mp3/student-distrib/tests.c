#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "idt.h"
#include "rtc.h"
#include "keyboard.h"
#include "terminal.h"
#include "file_systems.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 20 IDT entries are not NULL and tests RTC and Keyboard interrupts
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 20; i++){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
		if (idt[i].present == 0) {
			assertion_failure();
			result = FAIL;
		}
	}

	/* tests RTC and keyboard  being present */
	if (((idt[0x21].offset_15_00 == NULL) && (idt[0x21].offset_31_16 == NULL)) || idt[0x21].present == 0) {
		assertion_failure();
		result = FAIL;
	}

	if (((idt[0x28].offset_15_00 == NULL) && (idt[0x28].offset_31_16 == NULL)) || idt[0x28].present == 0) {
		assertion_failure();
		result = FAIL;
	}

	return result;

}

/*
 * system_call_handler_test
 *
 * Tests the system call vector
 * Inputs: None
 * Outputs: None
 * Side Effects: Halts the system and shows the fault message
 * Coverage: Exception/system call handling
 * Files: idt.c, exceptions.S
 */
int system_call_handler_test() {
    TEST_HEADER;

    __asm__("int    $0x80");
    return FAIL;
}

/*
 * divide_error_test
 * 
 * Tests first exception which is the divide error
 * Inputs: None
 * Outputs: None
 * Side Effects: Halts the system and shows fault message
 * Coverage: Exception handling
 * Files: idt.c, exceptions.S
 */
int divide_error_test() {
	TEST_HEADER;

	int x = 50;
	int y = 0;
	int z;
	z = x/y;
	return FAIL;
}

void exception_test() {
	asm("int $0x0C");
}

/*
 * paging_test
 *
 * Checks the kernel and video memory addresses to see if they exist
 * Inputs: None
 * Outputs: Pass/Fail
 * Side Effects: Halts the system
 * Coverage: Kernel and video memory within physical memory
 * Files: paging.c
 */
int paging_test() {
    TEST_HEADER;

    char result;
		char* addr = (char*)0xB8000;        //Video memory address
    result = *addr;

    addr = (char*)0x400000;     			//Kernel memory
    result = *addr;

    addr = (char*)0x7FFFFF;                 //Bottom of kernel memory
    result = *addr;

    addr = (char*)0xB8FFF;                 //Bottom of video memory
    result = *addr;

    return PASS; 
}

/*
 * video_mem_upper_lower_test
 *
 * Checks memory before and after video memory to see if there is a page fault
 * Inputs: None
 * Outputs: Pass/Fail
 * Side Effects: None
 * Coverage: Checks for page fault
 * Files: paging.c
 */
int video_mem_upper_lower_test() {
    TEST_HEADER;

    char result;
    char* address = (char*) 0xB7FFF;
    result = *address;

		char result1;
    char* address1 = (char*) 0xB9000;
    result1 = *address1;

    return FAIL;
}

/*
 * kernel_upper_lower_test
 *
 * Checks memory before and after the kernel to see if there is a page fault
 * Inputs: None
 * Outputs: Pass/Fail
 * Side Effects: None
 * Coverage: Checks for page fault
 * Files: paging.c
 */
int kernel_upper_lower_test() {
    TEST_HEADER;

    char result;
    char* address = (char*)0x3FFFFF;
    result = *address;

		char result1;
    char* address1 = (char*)0x800000;
    result1 = *address1;

    return FAIL;
}

/*
 * location_zero_test
 *
 * Checks if location 0 causes a page fault
 * Inputs: None
 * Outputs: Pass/Fail
 * Side Effects: None
 * Coverage: Checks for page fault
 * Files: paging.c
 */
int location_zero_test() {
    TEST_HEADER;

    char result;
    char* address = (char*) 0;
    result = *address;
    return FAIL;
}

/*
 * pic_disable_irq_wrong_test
 *
 * This sends an invalid IRQ number to make sure that nothing happens and that keyboard and RTC interrupts are still accepted
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 * Coverage: wrong input for disable_irq
 * Files: i8259.c
 */
void pic_disable_irq_wrong_test(){
	disable_irq(50);
}

/*
 * pic_disable_irq_test
 *
 * This test sends a valid IRQ number for the keyboard and RTC
 * Inputs: None
 * Outputs: None
 * Side Effects: Masks keyboard and RTC interrupts
 * Coverage: correct inputs for disable_irq
 * Files: i8259.c
 */
void pic_disable_irq_test(){
	disable_irq(KEYBOARD_IRQ);
}

/*
 * pic_enable_irq_wrong_test
 *
 * This test sends an invalid IRQ number to see the response of enable_irq
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 * Coverage: Wrong input for enable_irq
 * Files: i8259.c
 */
void pic_enable_irq_wrong_test(){
	enable_irq(50);
}

/*
 * pic_enable_irq_test
 *
 * This test sends a valid IRQ number for the keyboard and RTC for enable_irq
 * Inputs: None
 * Outputs: None
 * Side Effects: Unmasks keyboard and RTC interrupts
 * Coverage: Correct inputs for enable_irq
 * Files: i8259.c
 */
void pic_enable_irq_test(){
	enable_irq(KEYBOARD_IRQ);
	enable_irq(RTC_IRQ);
}

void send_eoi_wrong_test() {
	send_eoi(50);
}

void send_eoi_test() {
	send_eoi(RTC_IRQ);
	send_eoi(KEYBOARD_IRQ);
}

/* Checkpoint 2 tests */
/*
 * rtc_read_write tests
 *
 * This test increases the frequency of the rtc by consequetive powers of 2
 * Inputs: None
 * Outputs: Outputs '1' at the rate of the clock
 * Side Effects: Sets frequency to desired frequencies
 * Coverage: Correct setting of frequency in rtc
 * Files: rtc.c
 */
int rtc_read_write_test() {
    TEST_HEADER;

    uint32_t i;
    uint32_t j;
    int32_t frequency = 0;

    frequency += rtc_open(NULL);
    for(i = 2; i <= 1024; i*=2) {
        frequency += rtc_write(NULL, &i, sizeof(uint32_t));
        printf("Testing: %d Hz\n[", i);
        for(j = 0; j < i; j++) {
            frequency += rtc_read(NULL, NULL, NULL);
            printf("1");
        }
        printf("]\n");
    }
    if(frequency == 0) {
        return PASS;
    } else {
        return FAIL;
    }
}

/*
 * terminal_test
 *
 * Outputs buffer of inputted charactes upon pressing enter
 * Inputs: size of buffer used
 * Outputs: Outputs buffer of inputted charactes upon pressing enter
 * Side Effects: None.
 * Coverage: Corrent implementation of terminal_read() and terminal_write()
 * Files: terminal.c, keyboard.c
 */
int terminal_test(int size){
	clear();
    TEST_HEADER;
	int retval;
    char test_buff[KB_BUF_SIZE];
	terminal_write(0, (char*)"TESTING\n", 9);

	 while(1){
		retval = terminal_read(0, test_buff, size);
        if (retval == -1) {
            return FAIL;
        }

		retval = terminal_write(0, test_buff, size);
        if (retval == -1) {
            return FAIL;
        }
    }
    return 0;
}


/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	//TEST_OUTPUT("idt_test", idt_test());
	// TEST_OUTPUT("divide_error_test", divide_error_test());
	/*
	while(1) {
		test_interrupts();
	}
	*/
	//exception_test();

	//TEST_OUTPUT("paging_test", paging_test());
	//TEST_OUTPUT("video_mem_upper_lower_test", video_mem_upper_lower_test());
  	//TEST_OUTPUT("kernel_upper_lower_test", kernel_upper_lower_test());
  	//TEST_OUTPUT("location_zero_test", location_zero_test());
	

	//pic_disable_irq_wrong_test();	
	//pic_disable_irq_test();		
	//pic_enable_irq_wrong_test();		
	//pic_enable_irq_test();	
	//send_eoi_wrong_test();		
	//send_eoi_test();	
	//printf("TEST SUCCESS\n");
	//printf("PRESS KEYBOARD TO TEST\n");

	/* _________________CHECKPOINT 2 TESTS _________________________*/

	//TEST_OUTPUT("rtc_read_write_test", rtc_read_write_test());
	//TEST_OUTPUT("terminal_driver_test", terminal_test(128));

}
