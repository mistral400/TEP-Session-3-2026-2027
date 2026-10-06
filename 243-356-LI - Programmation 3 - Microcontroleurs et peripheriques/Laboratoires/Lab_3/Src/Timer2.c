#include "Timer2.h"

#define RCC_APB1ENR (*(volatile uint32_t *)0x4002101CUL)

#define TIM2_CR1  (*(volatile uint32_t *)0x40000000UL)
#define TIM2_DIER (*(volatile uint32_t *)0x4000000CUL)
#define TIM2_SR   (*(volatile uint32_t *)0x40000010UL)
#define TIM2_EGR  (*(volatile uint32_t *)0x40000014UL)
#define TIM2_CNT  (*(volatile uint32_t *)0x40000024UL)
#define TIM2_PSC  (*(volatile uint32_t *)0x40000028UL)
#define TIM2_ARR  (*(volatile uint32_t *)0x4000002CUL)

#define NVIC_ISER0 (*(volatile uint32_t *)0xE000E100UL)
#define NVIC_ICER0 (*(volatile uint32_t *)0xE000E180UL)
#define NVIC_ICPR0 (*(volatile uint32_t *)0xE000E280UL)

#define TIM2_CLOCK_ENABLE (1UL << 0)
#define TIM2_CEN           (1UL << 0)
#define TIM2_URS           (1UL << 2)
#define TIM2_UIE           (1UL << 0)
#define TIM2_UIF           (1UL << 0)
#define TIM2_UG            (1UL << 0)
#define TIM2_IRQ           (1UL << 28)

#define SECONDS_PER_DAY 86400UL

static volatile uint32_t secondesDepuisMinuit = 0UL;
static volatile uint32_t nombreDeMisesAJour = 0UL;

void Timer2_Init(void)
{
    NVIC_ICER0 = TIM2_IRQ; // désactive temporairement l'IRQ 28

    RCC_APB1ENR |= TIM2_CLOCK_ENABLE; // active l'horloge de TIM2

    TIM2_CR1 = TIM2_URS; // seules les fins de période demandent une IRQ
    TIM2_DIER = 0UL; // désactive les interruptions du timer
    TIM2_CNT = 0UL;
    TIM2_PSC = 7999UL; // 8 MHz / (7999 + 1) = 1 kHz
    TIM2_ARR = 999UL; // 1000 ticks à 1 kHz = 1 seconde

    TIM2_EGR = TIM2_UG; // applique le prescaler et le rechargement
    TIM2_SR = 0UL; // efface les indicateurs en attente
    TIM2_CNT = 0UL;
    nombreDeMisesAJour = 0UL;

    NVIC_ICPR0 = TIM2_IRQ; // efface une ancienne IRQ en attente
    TIM2_DIER = TIM2_UIE; // active l'interruption Update
    NVIC_ISER0 = TIM2_IRQ; // active l'IRQ 28 de TIM2
    TIM2_CR1 = TIM2_URS | TIM2_CEN; // démarre le compteur
}

uint32_t Timer2_GetSecondsSinceMidnight(void)
{
    return secondesDepuisMinuit;
}

uint32_t Timer2_GetUpdateCount(void)
{
    return nombreDeMisesAJour;
}

unsigned char Timer2_SetTime(uint8_t heures, uint8_t minutes, uint8_t secondes)
{
    if ((heures >= 24U) || (minutes >= 60U) || (secondes >= 60U))
    {
        return 0U;
    }

    secondesDepuisMinuit = ((uint32_t)heures * 3600UL)
                         + ((uint32_t)minutes * 60UL)
                         + (uint32_t)secondes;
    return 1U;
}

void TIM2_IRQHandler(void)
{
    if ((TIM2_SR & TIM2_UIF) == 0UL)
    {
        return;
    }

    TIM2_SR = 0UL; // efface l'indicateur Update

    if (secondesDepuisMinuit >= (SECONDS_PER_DAY - 1UL))
    {
        secondesDepuisMinuit = 0UL;
    }
    else
    {
        secondesDepuisMinuit++;
    }

    nombreDeMisesAJour++;

    /* Réveille la boucle principale pour afficher la nouvelle heure. */
    __asm volatile ("sev" ::: "memory");
}
