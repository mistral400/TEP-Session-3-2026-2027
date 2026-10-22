#include "LCD.h"
#include "Timer2.h"

#define RCC_CR   (*(volatile unsigned int *)0x40021000UL)
#define RCC_CFGR (*(volatile unsigned int *)0x40021004UL)

#define HSI_ON       (1u << 0u)
#define HSI_READY    (1u << 1u)

#define SYSCLK_SW    (3u << 0u)
#define SYSCLK_SWS   (3u << 2u)

#define AHB_PRE      (0xFu << 4u)
#define APB1_PRE     (7u << 8u)
#define APB2_PRE     (7u << 11u)

#define SET_HOURS    23U 
#define SET_MINUTES  59U
#define SET_SECONDS  55U 

void SystemInit(void)
{
    RCC_CR |= HSI_ON;

    while ((RCC_CR & HSI_READY) == 0u)
    {
    }

    RCC_CFGR &= ~SYSCLK_SW;

    while ((RCC_CFGR & SYSCLK_SWS) != 0u)
    {
    }

    RCC_CFGR &= ~(AHB_PRE | APB1_PRE | APB2_PRE);
}

int main(void) 
{
    unsigned int dernierAffichage; // nombre de mises à jour du timer lors du dernier affichage
    unsigned int nombreDeMisesAJour; // nombre de mises à jour du timer lors de l'itération en cours

    LCD_Init(); 

   Timer2_SetTime(SET_HOURS, SET_MINUTES, SET_SECONDS);


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
    }
}