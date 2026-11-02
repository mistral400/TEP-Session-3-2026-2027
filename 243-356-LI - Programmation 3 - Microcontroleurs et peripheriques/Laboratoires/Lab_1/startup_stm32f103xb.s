.syntax unified
.cpu cortex-m3
.thumb

.section .isr_vector,"a",%progbits
.globl g_pfnVectors
.type g_pfnVectors, %object
g_pfnVectors:
    .word _estack
    .word Reset_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler

.text
.thumb_func
.globl Reset_Handler
.type Reset_Handler, %function
Reset_Handler:
    ldr r0, =_sdata
    ldr r1, =_edata
    ldr r2, =_sidata
    cmp r0, r1
    beq .L_bss_init
.L_copy_loop:
    cmp r0, r1
    beq .L_bss_init
    ldr r3, [r2], #4
    str r3, [r0], #4
    b .L_copy_loop
.L_bss_init:
    ldr r0, =_sbss
    ldr r1, =_ebss
    mov r2, #0
.L_bss_loop:
    cmp r0, r1
    beq .L_call_main
    str r2, [r0], #4
    b .L_bss_loop
.L_call_main:
    bl main
    b .

.thumb_func
.globl Default_Handler
.type Default_Handler, %function
Default_Handler:
    b .

.size Reset_Handler, .-Reset_Handler
.size Default_Handler, .-Default_Handler
