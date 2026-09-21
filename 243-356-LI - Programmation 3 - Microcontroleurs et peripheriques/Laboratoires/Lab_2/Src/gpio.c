#include "gpio.h"

#define RCC_APB2ENR (*(volatile unsigned int *)0x40021018UL)

#define GPIOB_CRL   (*(volatile unsigned int *)0x40010C00UL)
#define GPIOB_IDR   (*(volatile unsigned int *)0x40010C08UL)
#define GPIOB_BSRR  (*(volatile unsigned int *)0x40010C10UL)

#define LED_PIN     6u
#define BUTTON_PIN  5u

#define LED_MASK    (1u << LED_PIN)
#define BUTTON_MASK (1u << BUTTON_PIN)

static unsigned char etatLED = LED_OFF;

void LED_Init(void)
{
    RCC_APB2ENR |= (1u << 3u); // active GPIOB

    LED_Set(LED_OFF); // commence éteinte

    GPIOB_CRL &= ~(0xFu << 24u); // efface PB6
    GPIOB_CRL |= (0x2u << 24u); // sortie push-pull 2 MHz
}

void LED_Set(unsigned char etat)
{
#if LED_ACTIVE_LOW == 1u

    if (etat == LED_ON)
    {
        GPIOB_BSRR = (LED_MASK << 16u); // PB6 à 0
    }
    else
    {
        GPIOB_BSRR = LED_MASK; // PB6 à 1
    }

#else

    if (etat == LED_ON)
    {
        GPIOB_BSRR = LED_MASK; // PB6 à 1
    }
    else
    {
        GPIOB_BSRR = (LED_MASK << 16u); // PB6 à 0
    }

#endif

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
    RCC_APB2ENR |= (1u << 3u); // active GPIOB

    GPIOB_CRL &= ~(0xFu << 20u); // efface PB5

#if BUTTON_INTERNAL_PULL == 0u

    GPIOB_CRL |= (0x4u << 20u); // entrée flottante

#else

    GPIOB_CRL |= (0x8u << 20u); // entrée pull-up/down

#if BUTTON_INTERNAL_PULL == 1u
    GPIOB_BSRR = BUTTON_MASK; // pull-up
#else
    GPIOB_BSRR = (BUTTON_MASK << 16u); // pull-down
#endif

#endif
}

unsigned char Button_Read(void)
{
    unsigned char bouton;

    bouton = (GPIOB_IDR & BUTTON_MASK) ? 1u : 0u; // lit PB5

#if BUTTON_ACTIVE_LOW == 1u
    bouton = !bouton; // appuyé = 1
#endif

    return bouton;
}