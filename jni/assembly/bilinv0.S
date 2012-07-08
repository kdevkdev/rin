    @ include constants
#include "../inc/pgshared.h"

    @ enable unified syntax (arm, thumb, thumb2)
.syntax unified 

    @ begin of text section
.text

    @ make pgBitBlitBilinearS globally accessible
    @ c signature: void pgBitBlitBilinearS(unsigned int w, unsigned int h, unsigned int dw, unsigned int dh, void * d, void * vptr);
.global pgBitBlitBilinearS

pgBitBlitBilinearS:
    @ Stack the return address (lr)
    @ keep the stack 8-byte aligned. (means even numbers of register, here 9+1)
    push    {r4-r12, lr}

    @ Get the 5 & 6 arguments into registers 7 and 8 (sourcevideopointer and framebufferpointer)
    ldr r7, [sp,#44] @ WARNING: this number depends on the numbers of registers pushed above, so adjust it if necessary
    ldr r8, [sp,#40]

    @ Move r0, r1 and r2 to r4, r5, and r6 to free the first three to registers for printf arguments
    mov r4, r0
    mov r5, r1
    mov r6, r2

    @ Move the destination height (dh) into r11 (r3 will be messed up by printf)
    mov r11, r3

    @ Horicontal loop, loop variable is in r9 for now 
    mov r9,  #0
loopy:

    @ Vertical loop, loop variable is in r10 for now
    mov r10, #0
loopx:
    @ Load the message for printf
    ldr     r0, =message

    @ Set two parameters
    mov r1, r10
    mov r2, r9
    bl      printf

    add r10, #1
    cmp r10, r6
    bls loopx

    add r9, #1
    cmp r9, r11
    bls loopy

    @ Exit from 'main'. Set return value to 0. 
    mov     r0, #0      @ Return 0.

    @ Return original register values, as well as push the return address into pc to return 
    pop     {r4-r12, pc}

    @ --------------------------------
    @ Data for the printf call. The GNU assembler's ".asciz" directive
    @ automatically adds a NULL character termination.
.data

message:
    .asciz  MSG @ some predefined string
