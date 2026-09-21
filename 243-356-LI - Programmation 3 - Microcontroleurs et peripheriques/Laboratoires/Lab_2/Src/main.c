#include "gpio.h"
#include "SysTick.h"
#include "Timer2.h"

#define MODE_0 0u
#define MODE_1 1u
#define MODE_2 2u

#define TEMPS_LED 500u
#define DEBOUNCE 20u

#define RCC_CR   (*(volatile unsigned int *)0x40021000UL)
#define RCC_CFGR (*(volatile unsigned int *)0x40021004UL)

void SystemInit(void)
{
    RCC_CR |= 1u; // active HSI

    while ((RCC_CR & (1u << 1u)) == 0u)
    {
    }

    RCC_CFGR &= ~3u; // prend HSI

    while ((RCC_CFGR & (3u << 2u)) != 0u)
    {
    }

    RCC_CFGR &= ~((0xFu << 4u) | (7u << 8u) | (7u << 11u)); // tout reste à 8 MHz
}

int main(void)
{
    unsigned char mode = MODE_0;
    unsigned char bouton;
    unsigned char ancienBouton;
    unsigned char boutonStable;

    unsigned int tempsBouton = 0u;
    unsigned int tempsLED = 0u;
    unsigned int temps;

    LED_Init();
    Button_Init();
    Timer2_Stop();
    SysTick_Init();

    bouton = Button_Read();
    ancienBouton = bouton;
    boutonStable = bouton;

    LED_Set(LED_ON); // commence allumé

    while (1)
    {
        temps = SysTick_GetMs();
        bouton = Button_Read();

        if (bouton != ancienBouton)
        {
            ancienBouton = bouton;
            tempsBouton = temps; // repart le debounce
        }

        if ((bouton != boutonStable) &&
            ((unsigned int)(temps - tempsBouton) >= DEBOUNCE))
        {
            boutonStable = bouton;

            if (boutonStable != 0u)
            {
                mode++; // prochain mode

                if (mode > MODE_2)
                {
                    mode = MODE_0;
                }

                Timer2_Stop();
                LED_Set(LED_OFF);

                if (mode == MODE_0)
                {
                    LED_Set(LED_ON);
                    tempsLED = temps;
                }
                else if (mode == MODE_1)
                {
                    Timer2_PWMInit();
                }
                else
                {
                    Timer2_InterruptInit();
                }
            }
        }

        if (mode == MODE_0)
        {
            if ((unsigned int)(temps - tempsLED) >= TEMPS_LED)
            {
                tempsLED = temps;
                LED_Toggle(); // change au 500 ms
            }
        }

        __asm volatile ("dsb\n\twfi" ::: "memory"); // attend prochaine interruption
    }
}