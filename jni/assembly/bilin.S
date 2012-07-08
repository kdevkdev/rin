    @ include constants
#include "../inc/pgshared.h"

    @ enable unified syntax (arm, thumb, thumb2)
.syntax unified 

    @ begin of text section
.text

    @ make pgBitBlitBilinearS globally accessible
    @ c signature: void pgBitBlitBilinearS(void * d, void * vptr, int comb_wx, int comb_wy);
.global pgBitBlitBilinearS

pgBitBlitBilinearS:
    @ Stack the return address (lr)
    @ keep the stack 8-byte aligned. (means even numbers of register, here 9+1)
    push    {r4-r12, lr}


    @ Load pixel weight steps (precalculated by preprocessor)
    @ the weight loops  sum this up to one, more exactly it is counted downards from 1 to zero (shifted by 10 of course). Note that due to insuffiecient precission in certain cases (ie when the screen ratio is for 1 to three)
    @ it is necessary to check the wether the remaining difference to one is smaller than the weight step, a zero check is not sufficient, because 
    @ the loop var might never reach zero.
    @ It also isnt really necessary (the dimensions are usually not large enough in this application) to keep this insufficient precission in order to eventually ad a pixel.
    
    @ put both weight steps twice (in high and low half- word) in r7 and r8 respectively in order to use simd instructiosn for updating weights. Saves 2 registers in the end since
    @ wx, wxo, wy, wyo can be stored in 2 registers instead of 4 (one each)
    mov r7, r3
    mov r8, r2


    @ Load gameboy screen height (the two high bytes in r6 contain the loop variabl - WARNING: they bust be zero before)
    @ this trick - to keep two loop variables in one register - frees one register (r14 that is desperately needed for pixel calculation)
    ldr r6, =(GSCREEN_HEIGHT<<16)

gscreenheightloop:
    @load top pixels and the two bottom pixels from the gameboy framebuffer into r14 and r5 respectively
    #ldr r14, [r0]
    #ldr r5, [r0, #(GLINE_SIZE*2)]
  
    @ r9 is loop var (contains both wx and wxo), leave the higher half-word zero
    ldr r9, =FIX_ONE

weightloopy:
    @load top pixels and the two bottom pixels from the gameboy framebuffer into r14 and r5 respectively
    #ldr r14, [r0]
    #ldr r5, [r0, #(GLINE_SIZE*2)]

    @ Load gameboy screen width (the two low bytes in r6 contain the loop variable - WARNING: they bust be zero before)
    add r6, r6, #GSCREEN_WIDTH
gscreenwidthloop:
    @load top pixels and the two bottom pixels from the gameboy framebuffer into r14 and r5 respectively
    ldr r14, [r0]
    ldr r5, [r0, #(GLINE_SIZE*2)]

    @r10 is loop var (contains both wy and wyo), leave the higher half-word zero
    ldr r10, =FIX_ONE

weightloopx:

    @summary:
    @	 	r9  contains wy and wyo (hi and low part respectively)
    @		r10 contains wx and wxo (hi and low part respectively)


    @red------------------------
    @extract top left  (first 5 bits from the left) and multiply
    @multiply by the corresponding factors, store it in r2
    mov r11, r14, lsr #27
    @top right
    ubfx r12, r14, #11, #5

    @put both into one word
    add r12, r12, r11, lsl #16
    @multiply and add
    smuad r2, r10, r12

    @bottom left
    mov r11, r5, lsr #27
    @bottom right
    ubfx r12, r5, #11, #5

    @put both into one word
    add r12, r12, r11, lsl #16
    @multiply and add
    smuad r11, r10, r12
    
    @top and bottom put both into one word
    add r11, r2, r11, lsl #16
    @multiply and add
    smuad r2, r11, r9

    @green---------------------
    @top left
    ubfx r11, r14, #21, #6
    @top right
    ubfx r12, r14, #5, #6

    @put both into one word
    add r12, r12, r11, lsl #16
    @multiply and add
    smuad r3, r10, r12

    @bottom left
    ubfx r11, r5, #21, #6
    @bottom right
    ubfx r12, r5, #5, #6

    @put both into one word
    add r12, r12, r11, lsl #16
    @multiply and add
    smuad r11, r10, r12

    @top and bottom put both into one word
    add r11, r3, r11, lsl #16
    @multiply and add
    smuad r3, r11, r9


    @blue---------------------
    @top left
    ubfx r11, r14, #16, #5
    @top right
    ubfx r12, r14, #0, #5

    @put both into one word
    add r12, r12, r11, lsl #16
    @multiply and add
    smuad r4, r10, r12

    @bottom left
    ubfx r11, r5, #16, #5
    @bottom right
    ubfx r12, r5, #0, #5

    @put both into one word
    add r12, r12, r11, lsl #16
    @multiply and add
    smuad r11, r10, r12

    @top and bottom put both into one word
    add r11, r4, r11, lsl #16
    @multiply and add
    smuad r4, r11, r9

    @--------------------------------------------
    @in r2 to r4 are now the weighted averages of red, green and blue
    @pack themback into the two lower bytes of r11

    mov r2, r2, lsr #(SHIFT_AM*2)
    mov r3, r3, lsr #(SHIFT_AM*2)
    mov r11, r4, lsr #(SHIFT_AM*2)
    
    add r11, r11, r2, lsl #11
    add r11, r11, r3, lsl #5

    @store half word at vpointer, increment vpointer by 2
    strh r11, [r1], #2

    @update weights x with simd
    qadd16 r10, r10, r8

    @compare (shift left by 16 to move the comparands to the same byte)
    cmp r8, r10, lsl #16

    @jump if the two higher bytes in r10 are still greater than the two in r8. 
    @though the two lower bytes also contain values, that should not affect the correctness since
    @their significance is too low. 
    @because of limited precisision zero test doesnt work here (could triger one pass too late)
    ble weightloopx

    @update gameboy videopointer
    add r0, r0, #2

    @one x-pixel finished
    @subtract one from the first two lower bytes of r6
    sub r6, #1
    @shift result to temporary register (r2, used for red otherwise), zero flag should get set if is equal to zero
    movs r2, r6, lsl #16
    @jump to the beginning of the loop
    bne gscreenwidthloop

    @one pixel row finished
    @reset gameboy fb pointer to line start
    sub r0, r0, GSCREEN_WIDTH*2

    @update weights y with simd (similar to the x-weights)
    qadd16 r9, r9, r7
    @ompare (shift left by 16 to move the comparands to the same byte)
    cmp r7, r9, lsl #16

    @jump if r9 is greater equal than r7 
    @because of limited precisision zero test doesnt work here
    ble weightloopy

    @advance one line in the gameboy framebuffer
    add r0, r0, GLINE_SIZE*2

    @subtract one from the two higher bytes of r6
    sub r6, r6, #(1 << 16)
    @test if is zero with the help of a temporary register (r2, used for red otherwise)f
    movs r2, r6, lsr #16
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
