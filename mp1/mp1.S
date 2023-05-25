
.data					# section declaration

        # Useful offset constants for accessing members of a 
        # struct mp1_blink_struct structure
        LOCATION   = 0    
        ON_CHAR    = 2
        OFF_CHAR   = 3 
        ON_LENGTH  = 4
        OFF_LENGTH = 6
        COUNTDOWN  = 8
        STATUS     = 10
        NEXT       = 12

        STRUCT_SIZE = 16

# Pointer to head of list (initialized to NULL)
mp1_list_head:
        .long   0

.text					# section declaration

# Export the function symbol names

.global mp1_rtc_tasklet
.global mp1_ioctl

# void mp1_poke(void);
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %cl  - The byte you wish to write
#            %eax - Offset from the start of video memory that you wish
#                   to write to
#   Outputs: Text-mode video screen is written to at location %eax with
#            the byte in %cl
# Registers: Clobbers EDX

mp1_poke:
        
	movl    vmem_base_addr(,1),%edx
	movb    %cl,(%edx,%eax,1)
	ret

// This function is in charge of switching between 
// the two frames. The function has 6 labels associated with it- each
// serving a different function.
// The function passes in an unsigned long and returns out once function's
// list ends.
// It does not return/output a specific value or pointer into any register.

// The C prototype: void mp1 rtc tasklet (unsigned long);

// Registers:
// eax- Used for storing LOCATION when calling mp1_poke
// ebx- Unused
// ecx- %cl used for storing OFF/ON_CHAR when calling mp1_poke
// edx- Unused
// esi- Stores the mp1_list_head pointer, and the subsequent struct
// edi- Used as a temp when moving ON/OFF_LENGTH to COUNTDOWN
mp1_rtc_tasklet:
        pushl   %ebp                    /* CALLEE SET UP */
        movl    %esp, %ebp
        pushl   %ebx
        pushl   %esi
        pushl   %edi

        movl    mp1_list_head, %esi     /* Setting list head to ESI */

// This loop helps in checking the COUNTDOWN field and decrementing it
// till it reaches zero. It also NULL checks the stack and the COUNTDOWN field.
// Once zero, the function moves on to tasklet_statuscheck. If not, it moves
// onto the next struct, and rechecks COUNTDOWN.
tasklet_loop:                           
        cmpl    $0, %esi                /* NULL check */
        je      tasklet_ret
        decw    COUNTDOWN(%esi)
        xorl    %eax, %eax
        cmpw    $0, COUNTDOWN(%esi)     /* NULL check */
        je      tasklet_statuscheck

tasklet_move:
        cmpl    $0, NEXT(%esi)          /* NULL check */
        je      tasklet_ret
        movl    NEXT(%esi), %esi        /* Moving to next struct */
        jmp     tasklet_loop

// This label checks the STATUS field once COUNTDOWN has reached zero.
// If it's zero, it jumps to tasklet_status0, which instructs the code
// on what to do. If not, it jumps to tasklet_status1, which does the
// for the other case.
tasklet_statuscheck:
        cmpw    $0, STATUS(%esi)        /* NULL check */
        je      tasklet_status0
        jmp     tasklet_status1

// This functions tells the code what to do when STATUS == 0. It calls
// mp1_poke by moving ON_CHAR to %cl, and the LOCATION to %eax- outputting
// the I/M characters to the screen respectively. It also updates COUNTDOWN
// setting it to ON_LENGTH, and jumping back to tasklet_move when done.
tasklet_status0:
        xorw    $1, STATUS(%esi)        /* Switching 0 to 1 */
        movb    ON_CHAR(%esi), %cl      /* Setting params for mp1_poke */
        movzwl  LOCATION(%esi), %eax
        shll    $1, %eax
        call    mp1_poke                /* Calling mp1_poke */
        movw    ON_LENGTH(%esi), %di    /* Updating COUNTDOWN */
        movw    %di, COUNTDOWN(%esi)
        jmp     tasklet_move

// This functions tells the code what to do when STATUS == 1. It calls
// mp1_poke by moving OFF_CHAR to %cl, and the LOCATION to %eax- outputting
// the I/M characters to the screen respectively. It also updates COUNTDOWN
// setting it to OFF_LENGTH, and jumping back to tasklet_move when done.
tasklet_status1:
        xorw    $1, STATUS(%esi)        /* Switching 1 to 0 */
        movb    OFF_CHAR(%esi), %cl     /* Pushing params for mp1_poke */
        movzwl  LOCATION(%esi), %eax
        shll    $1, %eax
        call    mp1_poke                /* Calling mp1_poke */
        movw    OFF_LENGTH(%esi), %di   /* Updating COUNTDOWN */
        movw    %di, COUNTDOWN(%esi)
        jmp     tasklet_move

// Special return function used for the tasklet function. This is special
// because I am not setting anything in %eax, while returning. All I did
// was pop off the callee-saved registers. 
tasklet_ret:
        popl    %edi                    /* CALLEE TEARDOWN */
        popl    %esi
        popl    %ebx
        leave
        ret

// This function passes in two arguments: a cmd and arg. The cmd value tells you 
// which of the four functions to jump to (add, remove, find and sync), like a
// dispatcher. The arg serves as the argument of each of the four functions I
// implemented- being the second thing on the stack.
// I use this argument as 'inputs' to each function- accessible by accessing 8 bytes 
// below the stack pointer. I utilize a jump table in order to jump between the four
// functions which is implemented below.
// The function returns/outputs -1 in the case where the cmd is out of bounds.

// The C Prototype: int mp1_ioctl (unsigned long arg, unsigned long cmd);

// Registers:
// eax- Used to store return address
// ebx- Unused
// ecx- Used to hold COUNTDOWN
// edx- Unused
// esi- Unused
// edi- Unused
mp1_ioctl:
        xorl    %ecx, %ecx              /* Initializing ecx */
        movl    8(%esp), %ecx           /* Moving COUNTDOWN to ecx */
        cmpl    $3, %ecx                /* Bound checks */
                                        /* MAGIC NUMBER- using 3 as that's the end
                                           of the given bounds of cmd of 0-3 */
        jg      ioctl_minus_one
        cmpl    $0, %ecx
        jl      ioctl_minus_one
        jmp     *jump_table(, %ecx, 4)  /* Jump to next function by jumptable*/

// Return -1 sequence just for mp1_ioctl. I implemented this seperately because
// I am not accessing any callee-saved registers which did not require them to
// be pushed on or popped off. All it does is shift -1 into %eax and ret.
ioctl_minus_one:
        movl    $-1, %eax               /* Return -1 case for mp1_ioctl */
        leave
        ret

// This function passes in a pointer to mp1_blink_struct. Its function is to help
// us display fishy to the screen. To do this, I call mp1_malloc to allocate necessary
// space to copy the struct onto, after which it is copied into the kernel space by
// using mp1_copy_from_user. After boundary and NULL checking, I update some of the
// struct's fields and then insert it into the linked list. I then call mp1_poke
// to help output fishy onto the display. I also call mp1_free when freeing memory
// allocated by mp1_malloc if it is out of bounds.
// The function returns/outputs 0 upon success, and -1 upon failure.

// The C Prototype: int mp1 ioctl add(unsigned long arg);

// Registers:
// eax- Used to store return address of each function call
// ebx- Used to store mp1_list_head from stack, 
// ecx- Used as a temp register to store and set the struct's fields. %cl is also
//      used to hold ON/OFF_CHAR info when calling mp1_poke
// edx- Unused
// esi- Used to store a pointer to the struct
// edi- Unused
mp1_ioctl_add:
        pushl   %ebp                    /* CALLEE SET UP */
        movl    %esp, %ebp
        pushl   %ebx
        pushl   %esi
        pushl   %edi

        movl    8(%ebp), %ebx           /* Pushing argument into ebx from stack*/
        cmpl    $0, %ebx
        je      ret_minus1
        pushl   $STRUCT_SIZE            /* Pushing struct size */
        call    mp1_malloc              /* Calling mp1_malloc */
        addl    $4, %esp
        cmpl    $0, %eax                /* NULL check */
        je      ret_minus1

        movl    %eax, %esi              /* Moving struct ptr to ESI */
        pushl   $STRUCT_SIZE            /* Pushing arguments onto stack */
        pushl   %ebx
        pushl   %esi
        call    mp1_copy_from_user      /* Calling mp1_copy_from_user */
        addl    $12, %esp
        cmpl    $0, %eax                /* Bound and NULL check */
        jne     free_memory
        cmpw    $0, LOCATION(%esi)
        jb      free_memory
        cmpw    $1999, LOCATION(%esi)   /* MAGIC NUMBER- Using (80x25)-1 as that is
                                           the end of the struct size */
        ja      free_memory

        movw    ON_LENGTH(%esi), %cx    /* Moving COUNTDOWN TO ON_LENGTH */
        movw    %cx, COUNTDOWN(%esi)
        movw    $1, STATUS(%esi)        /* Setting STATUS to 1 */

        movl    mp1_list_head, %ecx     /* Linking two structs*/
        movl    %ecx, NEXT(%esi)
        movl    %esi, mp1_list_head

        movb    ON_CHAR(%esi), %cl      /* Setting up registers for mp1_poke */
        movzwl  LOCATION(%esi), %eax
        shll    $1, %eax
        call    mp1_poke                /* Calling mp1_poke */
        jmp     ret_0


// This function takes in an integer location, which we use to traverse
// the linked list to find a field that matches its location. We remove
// it using mp1_free if found.
// The function has a jump, associated with an extra label, which handles
// removing structs if the location matches a struct's head.
// The function calls the helper function linked_list_search to locate 
// matching LOCATION fields.
// The function returns/outputs 0 upon success, and -1 upon failure.

// The C Prototype: int mp1 ioctl remove(unsigned long arg);

// Registers:
// eax- Used to store return address of each function call
// ebx- Used to store argument from the stack
// ecx- Used as a temp register to move and store struct fields.
// edx- Used to store pointer of previous stack after calling
//      linked_list_search
// esi- Used to store pointer of found location after calling
//      linked_list_search
// edi- Unused
mp1_ioctl_remove:
        pushl   %ebp                    /* CALLEE SETUP */
        movl    %esp, %ebp
        pushl   %ebx
        pushl   %esi
        pushl   %edi

        movl    8(%ebp), %ebx           /* Storing argument from stack to ebx */
        pushl   %ebx                    /* Pushing param for linked_list_search */
        call    linked_list_search      /* Calling linked_list_search */
        addl    $4, %esp
        cmpl    $-1, %eax               /* Bound check */
        je      ret_minus1

        movl    %eax, %esi              /* Storing ptr to esi */
        cmpl    mp1_list_head, %esi     /* Check if it is list head */
        je      remove_list_top

        movl    NEXT(%eax), %ecx        /* Linking previous struct to next struct */
        movl    %ecx, NEXT(%edx)
        pushl   %esi                    /* Pushing param for mp1_free if not list head*/
        call    mp1_free                /* Calling mp1_free */
        addl    $4, %esp
        jmp     ret_0

// This label takes care of removing a struct when LOCATION
// is found at the top of a struct. It sets the NEXT location
// of the previous node to the list's head, and call mp1_free
// to deallocate memory.
remove_list_top:
        movl    NEXT(%esi), %ecx        /* Linking previous struct to next struct */
        movl    %ecx, mp1_list_head
        pushl   %esi                    /* Pushing param for mp1_free */
        call    mp1_free                /* Calling mp1_free */
        addl    $4, %esp
        jmp     ret_0


// This function takes in a pointer to mp1_blink_struct, and like mp1_ioctl_add,
// requires us to allocate memory and copy the struct to kernel space. After which,
// mp1_copy_to_user is called to copy back the struct to user space, after calling
// the helper function linked_list_search if a location is found.
// NULL and out of bound testing included calling mp1_free to free memory allocated
// by calling mp1_malloc.
// The function returns/outputs 0 upon success, and -1 upon failure.
// The function also treats the pointer to mp1_blink_struct as an output, making
// it serve as both an input and an output

// The C Prototype: int mp1 ioctl find(unsigned long arg);

// Registers:
// eax- Used to store return address of each function call
// ebx- Used to store the pointer to the struct mp1_blink_struct.
// ecx- Used to store a ptr to the LOCATION field of the struct.
//      Pushed when calling linked_list_search
// edx- Unused
// esi- Used to store a pointer to the struct
// edi- Unused
mp1_ioctl_find:
        pushl   %ebp                    /* CALLEE SET UP */
        movl    %esp, %ebp
        pushl   %ebx
        pushl   %esi
        pushl   %edi

        movl    8(%ebp), %ebx           /* Pushing argument into ebx from stack*/
        cmpl    $0, %ebx
        je      ret_minus1
        pushl   $STRUCT_SIZE            /* Pushing struct size */
        call    mp1_malloc              /* Calling mp1_malloc */
        addl    $4, %esp
        cmpl    $0, %eax                /* NULL check */
        je      ret_minus1

        movl    %eax, %esi              /* Moving struct ptr to ESI */
        pushl   $STRUCT_SIZE            /* Pushing arguments onto stack */
        pushl   %ebx
        pushl   %esi
        call    mp1_copy_from_user      /* Calling mp1_copy_from_user */
        addl    $12, %esp
        cmpl    $0, %eax                /* Bound and NULL checks */
        jne     free_memory
        cmpw    $0, LOCATION(%esi)
        jb      free_memory
        cmpw    $1999, LOCATION(%esi)   /* MAGIC NUMBER- Using (80x25)-1 as that is
                                           the end of the struct size */
        ja      free_memory

        movzwl  LOCATION(%esi), %ecx    /* Moving LOCATION field to ESI */
        pushl   %ecx                    /* Pushing params */
        call    linked_list_search      /* Calling linked_list_search */
        addl    $4, %esp

        pushl   $STRUCT_SIZE            /* Pushing params */
        pushl   %eax
        pushl   %ebx
        call    mp1_copy_to_user        /* Calling mp1_copy_to_user */
        addl    $12, %esp
        cmpl    $0, %eax                /* Bound and NULL check */
        jne     free_memory
        cmpw    $0, LOCATION(%esi)
        jb      free_memory
        cmpw    $1999, LOCATION(%esi)
        ja      free_memory
        jmp     ret_0

// A helper function impemented to perform a linked list search to 
// return a pointer to see if we found the argument passed in. It has two
// labels associated with it: to help with comparisons and to traverse the
// linked list. This helper function is used in the mp1_ioctl_sync,
// mp1_ioctl_find and mp1_ioctl_remove functions.
// The input to this function is a pointer to the struct we wish to traverse,
// while the output is a pointer to a struct with a LOCATION field matching
// the input pushed in by the user.

// This function does use C calling convention as we are pushing/popping
// arguments on/off the stack. Since there is only one argument, little 
// endianness can be ignored for now.

// Registers:
// eax- Unused
// ebx- Used to store the pointer to the struct's head mp1_list_head
// ecx- Unused
// edx- Used to store a pointer to the previous struct
// esi- Used to store a pointer to the struct
// edi- Unused
linked_list_search:
        pushl   %ebp                    /* CALLEE SET UP */
        movl    %esp, %ebp
        pushl   %ebx
        pushl   %esi
        pushl   %edi

        movl    8(%ebp), %esi           /* ESI stores struct */
        movl    mp1_list_head, %ebx     /* EBX stores list head */

// This loop compares the LOCATION field of the struct to the argument
// extracted from the stack. It performs a NULL check along with a direct
// compare function called. If found, the function initiates a callee
// teardown. If not, the function traverses to the next struct.
linked_list_comparison:
        cmpl    $0, %ebx                /* NULL check */
        je      ret_minus1
        cmpw    %si, LOCATION(%ebx)     /* LOCATION comparison */
        jne     linked_list_nextsearch

        movl    %ebx, %eax              /* Stores return ptr */
        popl    %edi                    /* CALLEE TEARDOWN */
        popl    %esi
        popl    %ebx
        leave
        ret

// This function traverses to the next struct while storing a ptr
// to the previous struct. Once done, it jumps back to linked_list_comparison
// to repeat the process.  
linked_list_nextsearch:
        movl    %ebx, %edx              /* Storing previous struct ptr */
        movl    NEXT(%ebx), %ebx        /* Linking previous and next structs */
        jmp     linked_list_comparison


// This function helps synchronize the I/M with the switching between the
// two frames. The function calls in one 32-bit integer with information
// about two numbers- accessed through moving different bytes on the stack.
// I call my helper function linked_list_search twice to help me find pointers
// to the LOCATION field of both integers. The function has two extra labels-
// helping with what to do depending on the STATUS field value. It also copies
// over the timing information from the first element to the other.
// I also call mp1_poke to output the synchronized outputs onto the screen.
// The input to this function is a 32-bit integer which stores the information
// of both integers we wish to work with in this function. This function returns
// or outputs -1 upon failure or 0 upon success. It also calls an output to mp1_poke
// which outputs the synced frames with the I/M characters.

// The C Prototype: int mp1 ioctl sync(unsigned long arg);

// Registers:
// eax- Used to store return addresses for bothb linked_list_search calls
//      Also used for passing ON/OFF_CHAR info to mp1_poke
// ebx- Used to store 2nd integer, then as a temp reg to copy timing information
// ecx- Used to store 1st integer, %cl later used to store LOCATION when calling
//      mp1_poke
// edx- Unused
// esi- Unused
// edi- Used to store return address after first linked_list_seach call
//      before %eax got overwritten by the second call
mp1_ioctl_sync:
        pushl   %ebp                    /* CALLEE SET UP */
        movl    %esp, %ebp
        pushl   %ebx
        pushl   %esi
        pushl   %edi

        xorl    %ecx, %ecx
        movw    8(%ebp), %cx            /* Storing 1st Int */

        xorl    %ebx, %ebx
        movw    10(%ebp), %bx           /* Storing 2nd Int */
                                        /* MAGIC NUMBER- Using 10 to access the second two
                                           bytes of the passed in parameter */

        xorl    %edi, %edi
        cmpl    $0, %ebx                /* NULL checks for both ints */
        je      ret_minus1
        cmpl    $0, %ecx
        je      ret_minus1

        pushl   %ecx                    /* Pushing param */
        call    linked_list_search      /* Calling linked_list_search on first int */
        addl    $4, %esp
        cmpl    $-1, %eax               /* NULL check */
        je      ret_minus1
        movl    %eax, %edi              /* Storing return address into EDI */

        pushl   %ebx                    /* Pushing param */
        call    linked_list_search      /* Calling linked_list_search on second int */
        addl    $4, %esp
        cmpl    $-1, %eax               /* NULL check */
        je      ret_minus1

        movw    ON_LENGTH(%eax), %bx    /* Copying timing information */
        movw    %bx, ON_LENGTH(%edi)
        movw    OFF_LENGTH(%eax), %bx
        movw    %bx, OFF_LENGTH(%edi)
        movw    COUNTDOWN(%eax), %bx
        movw    %bx, COUNTDOWN(%edi)
        movw    STATUS(%eax), %bx
        movw    %bx, STATUS(%edi)
        cmpw    $0, STATUS(%edi)        /* STATUS field check */
        jne     sync_status1

// Instructions if STATUS == 0:
// Moving OFF_CHAR to %cl and LOCATION to %eax to call mp1_poke
sync_status0:
        movb    OFF_CHAR(%edi), %cl     /* Moving arguments into required regs */
        movzwl  LOCATION(%edi), %eax
        shll    $1, %eax
        call    mp1_poke                /* Calling mp1_poke */
        jmp     ret_0

// Instructions if STATUS == 1:
// Moving ON_CHAR to %cl and LOCATION to %eax to call mp1_poke
sync_status1:
        movb    ON_CHAR(%edi), %cl      /* Moving arguments into required regs */
        movzwl  LOCATION(%edi), %eax
        shll    $1, %eax
        call    mp1_poke                /* Calling mp1_poke */
        jmp     ret_0


// The Jump Table used to help mp1_ioctl to jump between the four
// coded functions.
jump_table:
        .long mp1_ioctl_add, mp1_ioctl_remove, mp1_ioctl_find, mp1_ioctl_sync


// This is a helper function I created when NULL/Bound checking
// after calling mp1_malloc. By definition, it always returns -1.
free_memory:
        pushl   %esi                    /* Pushing struct argument */
        call    mp1_free                /* Call mp1_free */
        addl    $4, %esp
        jmp     ret_minus1

// Return -1 Sequence. This is a label I use whenever I wish to return -1
// in order to save time. I push all arugments in reverse order in all my
// functions to help me generalize a teardown function
ret_minus1:
        movl    $-1, %eax               /* Return -1 */
        popl    %edi                    /* CALLEE TEARDOWN */
        popl    %esi
        popl    %ebx
        leave
	ret

// Return 0 Sequence. This is a label I use whenever I wish to return 0
// in order to save time. I push all arugments in reverse order in all my
// functions to help me generalize a teardown function
ret_0:
        xorl    %eax, %eax              /* Return 0 */
        popl    %edi                    /* CALLEE TEARDOWN */
        popl    %esi
        popl    %ebx
        leave
	ret

// Return 1 Sequence. This is a label I use whenever I wish to return 1
// in order to save time. I push all arugments in reverse order in all my
// functions to help me generalize a teardown function
ret_1:
        movl    $1, %eax                /* Return 1 */
        popl    %edi                    /* CALLEE TEARDOWN */
        popl    %esi
        popl    %ebx
        leave
        ret

.end
