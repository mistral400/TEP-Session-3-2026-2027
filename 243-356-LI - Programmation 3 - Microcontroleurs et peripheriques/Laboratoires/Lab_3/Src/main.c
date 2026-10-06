#include "LCD.h"
#include "Timer2.h"

#define RCC_CR   (*(volatile unsigned int *)0x40021000UL)
#define RCC_CFGR (*(volatile unsigned int *)0x40021004UL)

void SystemInit(void)
{
    RCC_CR |= 1u; // active HSI

    while ((RCC_CR & (1u << 1u)) == 0u)
    {
    }

    RCC_CFGR &= ~3u; // choisit HSI comme horloge système

    while ((RCC_CFGR & (3u << 2u)) != 0u)
    {
    }

    RCC_CFGR &= ~((0xFu << 4u) | (7u << 8u) | (7u << 11u));
    // AHB, APB1 et APB2 restent divisés par 1; SYSCLK vaut 8 MHz.
}

int main(void)
{
    unsigned int dernierAffichage;
    unsigned int nombreDeMisesAJour;

    LCD_Init();
    (void)Timer2_SetTime(0U, 0U, 0U);
    LCD_Affichage_Temps();
    Timer2_Init();

    dernierAffichage = Timer2_GetUpdateCount();

    while (1)
    {
        nombreDeMisesAJour = Timer2_GetUpdateCount();

        if (nombreDeMisesAJour != dernierAffichage)
        {
            dernierAffichage = nombreDeMisesAJour;
            LCD_Affichage_Temps();
        }

        /* TIM2 envoie SEV à chaque seconde et réveille cette attente. */
        __asm volatile ("wfe" ::: "memory");
    }
}
