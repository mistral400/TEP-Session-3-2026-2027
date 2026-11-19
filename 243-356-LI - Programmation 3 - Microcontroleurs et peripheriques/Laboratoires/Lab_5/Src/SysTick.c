#include "SysTick.h"

#define SYST_CSR (*(volatile uint32_t *)0xE000E010UL)
#define SYST_RVR (*(volatile uint32_t *)0xE000E014UL)
#define SYST_CVR (*(volatile uint32_t *)0xE000E018UL)

#define ENABLE    (1u << 0u)
#define TICKINT   (1u << 1u)
#define CLKSOURCE (1u << 2u)

#define CPU_HZ 8000000UL
#define SYSTICK_HZ 1000UL
#define SYSTICK_RELOAD ((CPU_HZ / SYSTICK_HZ) - 1UL)
#define SYSTICK_PERIODE (SYSTICK_RELOAD + 1UL)
#define SYSTICK_DELAI_MAX_CYCLES (SYSTICK_PERIODE / 2UL)
#define US_PAR_SECONDE 1000000UL

static volatile uint32_t tempsMs = 0u;

static void SysTick_AttendreCycles(uint32_t cyclesAAttendre)
{
    uint32_t depart = SYST_CVR;
    uint32_t actuel;
    uint32_t cyclesEcoules;

    do
    {
        actuel = SYST_CVR;
        if (depart >= actuel)
        {
            cyclesEcoules = depart - actuel;
        }
        else
        {
            cyclesEcoules = depart + SYSTICK_PERIODE - actuel;
        }
    }
    while (cyclesEcoules < cyclesAAttendre);
}

void SysTick_Init(void)
{
    SYST_CSR = 0u; // arrête SysTick
    tempsMs = 0u;

    SYST_RVR = SYSTICK_RELOAD;
    SYST_CVR = 0u; // remet à zéro

    SYST_CSR = CLKSOURCE | TICKINT | ENABLE; // démarre SysTick
}

uint32_t SysTick_GetMs(void)
{
    return tempsMs;
}

void SysTick_Handler(void)
{
    tempsMs++; // ajoute 1 ms
}

void SysTick_DelaiUs(uint32_t microsecondes)
{
    uint32_t cyclesRestants = (CPU_HZ / US_PAR_SECONDE) * microsecondes;

    while (cyclesRestants > 0UL)
    {
        uint32_t cyclesAAttendre = cyclesRestants;

        if (cyclesAAttendre > SYSTICK_DELAI_MAX_CYCLES)
        {
            cyclesAAttendre = SYSTICK_DELAI_MAX_CYCLES;
        }

        SysTick_AttendreCycles(cyclesAAttendre);
        cyclesRestants -= cyclesAAttendre;
    }
}

void SysTick_DelaiMs(uint32_t millisecondes)
{
    uint32_t depart = SysTick_GetMs();

    while ((uint32_t)(SysTick_GetMs() - depart) < millisecondes)
    {
    }
}
