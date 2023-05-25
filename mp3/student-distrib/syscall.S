#define ASM     1

.align 4
system_call_jump:   # there isn't a 0th system call
    .long 0x0, halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn           

.globl system_call_handler
.align 4
system_call_handler:  # can't push eax register
    pushl   %ebp
    pushl   %ebx
    pushl   %esi
    pushl   %edi
    pushl   %ecx
    pushl   %esp
    pushl   %edx
    pushfl

    # pushes the three arguments in system calls
    pushl %edx
    pushl %ecx
    pushl %ebx

    # checks to see if correct system call
    cmpl    $1, %eax        # no system calls below 1
    jb      wrong
    cmpl    $10, %eax       # no system calls above 10
    ja      wrong

    # use jumptable to get to the right system call
    call    *system_call_jump(, %eax, 4)   # 4 bytes per entry in table
    jmp     correct

wrong:
    movl    $-1, %eax       # stores -1 in eax to indicate wrong system call

correct:
    addl    $12, %esp       # moves the stack pointer up 12 for the 3 arguments that was pushed
    popfl                   # popping registers
    popl    %edx
    popl    %esp
    popl    %ecx
    popl    %edi
    popl    %esi
    popl    %ebx
    popl    %ebp
    iret


# Flushing TLB. Changing cr3 to flush it
.globl flush_tlb
.align 4
flush_tlb:
    movl %cr3, %eax    
    movl %eax, %cr3
    ret

.globl halt_assembly
.align 4
halt_assembly:
    pushl   %ebp
    movl    %esp, %ebp
    xorl    %ebx, %ebx
    movl    16(%ebp), %ebx # Getting return arg
    movl    %ebx, %eax
    movl    12(%ebp), %ebx # Getting ESP
    movl    %ebx, %esp
    movl    8(%ebp), %ebx # Getting EBP
    movl    %ebx, %ebp
    leave
    ret
