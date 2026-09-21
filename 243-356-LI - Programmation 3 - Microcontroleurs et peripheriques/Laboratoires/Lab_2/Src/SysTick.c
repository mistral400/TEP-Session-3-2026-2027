#include "SysTick.h"

#define SYST_CSR (*(volatile unsigned int *)0xE000E010UL)
#define SYST_RVR (*(volatile unsigned int *)0xE000E014UL)
#define SYST_CVR (*(volatile unsigned int *)0xE000E018UL)

#define ENABLE    (1u << 0u)
#define TICKINT   (1u << 1u)
#define CLKSOURCE (1u << 2u)

static volatile unsigned int tempsMs = 0u;

void SysTick_Init(void)
{
    SYST_CSR = 0u; // arrête SysTick
    tempsMs = 0u;

    SYST_RVR = 7999u; // 1 ms avec 8 MHz
    SYST_CVR = 0u; // remet à zéro

    SYST_CSR = CLKSOURCE | TICKINT | ENABLE; // démarre SysTick
}

unsigned int SysTick_GetMs(void)
{
    return tempsMs;
}

void SysTick_Handler(void)
{
    tempsMs++; // ajoute 1 ms
}