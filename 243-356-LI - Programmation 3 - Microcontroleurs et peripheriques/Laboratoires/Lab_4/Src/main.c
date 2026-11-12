#include "ADC.h"
#include "LCD.h"

#include <stdint.h>

#define RCC_CR   (*(volatile uint32_t *)0x40021000UL)
#define RCC_CFGR (*(volatile uint32_t *)0x40021004UL)

#define HSI_ON       (1UL << 0U)
#define HSI_READY    (1UL << 1U)

#define SYSCLK_SW    (3UL << 0U)
#define SYSCLK_SWS   (3UL << 2U)

#define AHB_PRE      (0xFUL << 4U)
#define APB1_PRE     (7UL << 8U)
#define APB2_PRE     (7UL << 11U)

// Configure le LCD et les broches GPIO pour l'affichage
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
    LCD_Init();
    ADC1_Init();

    while (1)
    {
        LCD_Affichage_Potentiometre();
    }
}
// LA LA BTW LE MAIN EST FINI FAQUE ARRETE DE SCROLLER TU PERD TON TEMPS
