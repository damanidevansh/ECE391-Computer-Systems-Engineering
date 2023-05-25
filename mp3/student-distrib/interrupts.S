#define ASM

# assembly linkage for interrupts
#define INTERRUPT_HANDLER(name, handler)         \
.globl  name                                    ;\
name:                                           ;\
    pushal                                      ;\
    pushfl                                      ;\
    call    handler                             ;\
    popfl                                       ;\
    popal                                       ;\
    iret

INTERRUPT_HANDLER(keyboard_interrupt, keyboard_irq_handler);
INTERRUPT_HANDLER(RTC_interrupt, rtc_irq_handler);
INTERRUPT_HANDLER(PIT_interrupt, pit_irq_handler);
