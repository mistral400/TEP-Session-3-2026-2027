#include "System.h"

#include <stdint.h>

#define RCC_CR   (*(volatile uint32_t *)0x40021000UL)
#define RCC_CFGR (*(volatile uint32_t *)0x40021004UL)

#define RCC_HSI_ON        (1UL << 0U)
#define RCC_HSI_READY     (1UL << 1U)
#define RCC_SYSCLK_SWITCH (3UL << 0U)
#define RCC_SYSCLK_STATUS (3UL << 2U)
#define RCC_AHB_PRESCALER (15UL << 4U)
#define RCC_APB1_PRESCALER (7UL << 8U)
#define RCC_APB2_PRESCALER (7UL << 11U)
#define RCC_BUS_PRESCALERS (RCC_AHB_PRESCALER \
                          | RCC_APB1_PRESCALER \
                          | RCC_APB2_PRESCALER)

void SystemInit(void)
{
    RCC_CR |= RCC_HSI_ON;

    while ((RCC_CR & RCC_HSI_READY) == 0UL)
    {
    }

    RCC_CFGR &= ~RCC_SYSCLK_SWITCH;

    while ((RCC_CFGR & RCC_SYSCLK_STATUS) != 0UL)
    {
    }

    /* HSI 8 MHz sans division: SYSCLK, PCLK1 et PCLK2 valent 8 MHz. */
    RCC_CFGR &= ~RCC_BUS_PRESCALERS;
}
