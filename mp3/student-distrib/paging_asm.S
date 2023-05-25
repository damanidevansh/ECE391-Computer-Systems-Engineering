#define ASM     1

# sets the page directory
.text
.globl loadPageDirectory

loadPageDirectory:
    pushl %ebp
    movl  %esp, %ebp
    movl  8(%esp), %eax
    movl  %eax, %cr3
    movl  %ebp, %esp
    pop   %ebp
    ret

# sets the page entry table
.text
.globl enablePaging

enablePaging:
    pushl %ebp
    movl  %esp, %ebp
    movl  %cr4, %eax
    orl   $0x10, %eax
    movl  %eax, %cr4
    xorl  %eax, %eax
    movl  %cr0, %eax
    orl   $0x80000000, %eax
    movl  %eax, %cr0
    movl  %ebp, %esp
    pop   %ebp
    ret
