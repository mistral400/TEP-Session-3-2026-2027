
#include "gpio.h"

#define RCC_APB2ENR (*(volatile unsigned int *)0x40021018UL)

#define GPIOB_CRL  (*(volatile unsigned int *)0x40010C00UL)
#define GPIOB_IDR  (*(volatile unsigned int *)0x40010C08UL)
#define GPIOB_BSRR (*(volatile unsigned int *)0x40010C10UL)

#define LED_PIN     6u
#define BUTTON_PIN  5u

#define LED_MASK    (1u << LED_PIN)
#define BUTTON_MASK (1u << BUTTON_PIN)

#define GPIOB_CLOCK_ENABLE (1u << 3u)
#define GPIO_RESET_SHIFT   16u

#define GPIO_CONFIG_BITS   4u
#define GPIO_CONFIG_MASK   0xFu

#define LED_CONFIG_SHIFT    (LED_PIN * GPIO_CONFIG_BITS)
#define BUTTON_CONFIG_SHIFT (BUTTON_PIN * GPIO_CONFIG_BITS)

#define LED_OUTPUT_MODE  0x2u
#define BUTTON_INPUT_MODE 0x4u

static unsigned char etatLED = LED_OFF;

void LED_Init(void)
{
    RCC_APB2ENR |= GPIOB_CLOCK_ENABLE;

    GPIOB_CRL &= ~(GPIO_CONFIG_MASK << LED_CONFIG_SHIFT);
    GPIOB_CRL |= (LED_OUTPUT_MODE << LED_CONFIG_SHIFT);

    LED_Set(LED_OFF);
}

void LED_Set(unsigned char etat)
{
    if (etat == LED_ON)
    {
        GPIOB_BSRR = (LED_MASK << GPIO_RESET_SHIFT);
    }
    else
    {
        GPIOB_BSRR = LED_MASK;
    }

    etatLED = etat;
}

void LED_Toggle(void)
{
    if (etatLED == LED_OFF)
    {
        LED_Set(LED_ON);
    }
    else
    {
        LED_Set(LED_OFF);
    }
}

void Button_Init(void)
{
    RCC_APB2ENR |= GPIOB_CLOCK_ENABLE;

    GPIOB_CRL &= ~(GPIO_CONFIG_MASK << BUTTON_CONFIG_SHIFT);
    GPIOB_CRL |= (BUTTON_INPUT_MODE << BUTTON_CONFIG_SHIFT);
}

unsigned char Button_Read(void)
{
    if ((GPIOB_IDR & BUTTON_MASK) == 0u)
    {
        return 1u;
    }
    else
    {
        return 0u;
    }
}
