
#include "gpio.h" // Inclut les déclarations des fonctions et constantes GPIO

#include <stdint.h>

#define RCC_APB2ENR (*(volatile uint32_t *)0x40021018UL)
#define GPIOA_CRL (*(volatile uint32_t *)0x40010800UL)
#define GPIO_PA0_CONFIG_MASK 0x0FUL
#define GPIOA_CLOCK_ENABLE (1UL)

void GPIOA_ConfigurerPA0Analogique(void)
{
    RCC_APB2ENR |= GPIOA_CLOCK_ENABLE;
    GPIOA_CRL &= ~GPIO_PA0_CONFIG_MASK;
}
