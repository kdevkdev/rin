    @ include constants
#include "../inc/pgshared.h"

    @ enable unified syntax (arm, thumb, thumb2)
.syntax unified 

    @ begin of text section
.text

    @ make pgBitBlitBilinearS globally accessible
    @ c signature: void pgBitBlitBilinearS(void * d, void * vptr);
.global pgBitBlitBilinearS

pgBitBlitBilinearS:
    @ Stack the return address (lr)
    @ keep the stack 8-byte aligned. (means even numbers of register, here 9+1)
    push    {r4-r12, lr}


    @ Load pixel weight steps (precalculated by preprocessor)
    @ the weight loops  sum this up to one, more exactly it is counted downards from 1 to zero (shifted by 20 of course). Note that due to insuffiecient precission in certain cases (ie when the screen ratio is for 1 to three)
    @ it is necessary to check the wether the remaining difference to one is smaller than the weight step, a zero check is not sufficient, because 
    @ the loop var might never reach zero.
    @ It also isnt really necessary (the dimensions are usually not large enough in this application) to keep this insufficient precission in order to eventually ad a pixel.
    ldr r7, =BILIN_WSX
    ldr r8, =BILIN_WSY

    mov r6, #0

    @ Load gameboy screen height (the two high bytes in r6 contain the loop variabl - WARNING: they bust be zero before)
    @ this trick - to keep two loop variables in one register - frees one register (r14 that is desperately needed for pixel calculation)
    add r6, r6, #(GSCREEN_HEIGHT<<16)
gscreenheightloop:

    @ r9 and r11 are loop variables
    mov r9, #0
    ldr r11, =FIX_ONE

weightloopy:

    @ Load gameboy screen width (the two low bytes in r6 contain the loop variable - WARNING: they bust be zero before)
    add r6, r6, #GSCREEN_WIDTH
gscreenwidthloop:

    @r10 and r12 are loop variables
    mov r10, #0
    ldr r12, =FIX_ONE
weightloopx:
    @load top pixels from the gameboy framebuffer into r14
    ldr r14, [r0]
    
    @top left pixel
    @extract red (first 5 bits from the left) and multiply
    @multiply by the corresponding factors, store it in r2
    mov r5, r14, lsr #27
    mul r2, r5, r10

    @green (second 6 bits from the left)
    @mov r5, r14, lsl #5
    @mov r5, r5, lsr #26
    ubfx r5, r14, #21, #6
    mul r3, r5, r10


    @and blue (last 5 bits from the left)
    @mov r5, r14, lsl #11
    @mov r5, r5, lsr #27
    ubfx r5, r14, #16, #5
    mul r4, r5, r10

    @now the top right pixel (it is already loaded into the lower to bytes of r14; ldr is 32 bit)
    @red
    @mov r5, r14, lsl #16
    @mov r5, r5, lsr #27
    ubfx r5, r14, #11, #5
    mla r2, r5, r12, r2

    @green
    @mov r5, r14, lsl #21
    @mov r5, r5, lsr #26
    ubfx r5, r14, #5, #6
    mla r3, r5, r12, r3

    @blue
    @mov r5, r14, lsl #27
    @mov r5, r5, lsr #27
    ubfx r5, r14, #0, #5
    mla r4, r5, r12, r4

    @ multiply by the top-pixel line weight factor (wyo in r11)
    mul r2, r11, r2
    mul r3, r11, r3
    mul r4, r11, r4

    @load the two bottom pixels into r14*/
    ldr r14, [r0, #(SIZE_LINE*2)]
    
    @bottom left pixel
    @red
    mov r5, r14, lsr #27
    mul r5, r9, r5
    mla r2, r5, r10, r2

    @green (second 6 bits from the left)
    @mov r5, r14, lsl #5
    @mov r5, r5, lsr #26
    ubfx r5, r14, #21, #6
    mul r5, r9, r5
    mla r3, r5, r10, r3

    @and blue (last 5 bits from the left)
    @mov r5, r14, lsl #11
    @mov r5, r5, lsr #27
    ubfx r5, r14, #16, #5
    mul r5, r9, r5
    mla r4, r5, r10, r4

    @and as last the bottom right pixel
    @red
    @mov r5, r14, lsl #16
    @mov r5, r5, lsr #27
    ubfx r5, r14, #11, #5
    mul r5, r9, r5
    mla r2, r5, r12, r2

    @green 
    @mov r5, r14, lsl #21
    @mov r5, r5, lsr #26
    ubfx r5, r14, #5, #6
    mul r5, r9, r5
    mla r3, r5, r12, r3

    @blue
    @mov r5, r14, lsl #27
    @mov r5, r5, lsr #27
    ubfx r5, r14, #0, #5
    mul r5, r9, r5
    mla r4, r5, r12, r4

    @in r2 to r4 are now the weighted averages of red, green and blue
    @pack themback into 2 bytes 
    ldr r14, =0

    mov r2, r2, lsr #20
    mov r3, r3, lsr #20
    mov r14, r4, lsr #20
    
    add r14, r14, r2, lsl #11
    add r14, r14, r3, lsl #5

    @store half word at vpointer, increment vpointer by 2
    strh r14, [r1], #2

    @update weights x
    add r10, r10, r7
    subs r12, r12, r7
    cmp r12, r7
    @jump if r12 is greatger equal than r7 
    @because of limited precisision zero test doesnt work here
    bge weightloopx

    add r0, r0, #2

    @one x-pixel finished
    @subtract one from the first two lower bytes of r6
    sub r6, #1
    @shift result to temporary register, zero flag should get set if is equal to zero
    movs r14, r6, lsl #16
    @jump to the beginning of the loop
    bne gscreenwidthloop

    @one pixel row finished
    @reset gameboy fb pointer to line start
    sub r0, r0, GSCREEN_WIDTH*2

    @update weights y
    add r9, r9, r8
    subs r11, r11, r8	
    cmp r11, r8
    @jump if r11 is greater equal than r8 
    @because of limited precisision zero test doesnt work here
    bge weightloopy

    @advance one line in the gameboy framebuffer
    add r0, r0, SIZE_LINE*2

    @subtract one from the two higher bytes of r6
    sub r6, r6, #(1 << 16)
    @test if is zero with the help of a temporary register
    movs r14, r6, lsr #16
    bne gscreenheightloop

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
