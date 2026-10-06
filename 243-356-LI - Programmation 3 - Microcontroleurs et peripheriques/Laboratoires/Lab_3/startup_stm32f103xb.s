.syntax unified
.cpu cortex-m3
.thumb

.section .isr_vector,"a",%progbits
.globl g_pfnVectors
.type g_pfnVectors, %object
g_pfnVectors:
    .word _estack
    .word Reset_Handler
    .word Default_Handler // NMI
    .word Default_Handler // HardFault
    .word Default_Handler // MemManage
    .word Default_Handler // BusFault
    .word Default_Handler // UsageFault
    .word 0, 0, 0, 0 // réservés
    .word Default_Handler // SVC
    .word Default_Handler // DebugMon
    .word 0 // réservé
    .word Default_Handler // PendSV
    .word SysTick_Handler // exception 15
    .rept 28
    .word Default_Handler // IRQ 0 à 27
    .endr
    .word TIM2_IRQHandler // IRQ 28, vecteur 44
    .rept 14
    .word Default_Handler // IRQ 29 à 42 du STM32F103xB
    .endr
.size g_pfnVectors, .-g_pfnVectors

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
    ldr r0, =g_pfnVectors
    ldr r1, =0xE000ED08 // VTOR
    str r0, [r1]
    dsb
    isb
    bl SystemInit
    bl main
    b .

.size Reset_Handler, .-Reset_Handler

.thumb_func
.globl Default_Handler
.type Default_Handler, %function
Default_Handler:
    b .

.size Default_Handler, .-Default_Handler
