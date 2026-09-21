#include "Timer2.h"
#include "gpio.h"

#define RCC_APB1ENR (*(volatile unsigned int *)0x4002101CUL)

#define TIM2_CR1   (*(volatile unsigned int *)0x40000000UL)
#define TIM2_DIER  (*(volatile unsigned int *)0x4000000CUL)
#define TIM2_SR    (*(volatile unsigned int *)0x40000010UL)
#define TIM2_EGR   (*(volatile unsigned int *)0x40000014UL)
#define TIM2_CNT   (*(volatile unsigned int *)0x40000024UL)
#define TIM2_PSC   (*(volatile unsigned int *)0x40000028UL)
#define TIM2_ARR   (*(volatile unsigned int *)0x4000002CUL)

#define NVIC_ISER0 (*(volatile unsigned int *)0xE000E100UL)
#define NVIC_ICER0 (*(volatile unsigned int *)0xE000E180UL)
#define NVIC_ICPR0 (*(volatile unsigned int *)0xE000E280UL)

#define TIM2_EN    (1u << 0u)
#define CEN        (1u << 0u)
#define URS        (1u << 2u)
#define UIE        (1u << 0u)
#define UIF        (1u << 0u)
#define UG         (1u << 0u)

#define TIM2_IRQ   (1u << 28u)
#define PWM_STEPS  100u

static volatile unsigned char modeTimer = TIMER2_MODE_STOP;
static volatile unsigned char compteurPWM = 0u;

void Timer2_Stop(void)
{
    NVIC_ICER0 = TIM2_IRQ; // coupe IRQ TIM2

    RCC_APB1ENR |= TIM2_EN; // active horloge TIM2

    TIM2_CR1 = 0u; // arrête timer
    TIM2_DIER = 0u; // coupe interruptions
    TIM2_SR = 0u; // efface flags
    TIM2_CNT = 0u; // remet compteur à zéro

    NVIC_ICPR0 = TIM2_IRQ; // efface IRQ en attente

    modeTimer = TIMER2_MODE_STOP;
}

static void Timer2_Start(void)
{
    TIM2_CR1 = URS; // IRQ seulement en fin de période

    TIM2_EGR = UG; // charge PSC et ARR
    TIM2_SR = 0u; // efface UIF
    TIM2_CNT = 0u; // repart de zéro

    NVIC_ICPR0 = TIM2_IRQ; // efface ancienne IRQ
    TIM2_DIER = UIE; // active update interrupt
    NVIC_ISER0 = TIM2_IRQ; // active IRQ TIM2

    TIM2_CR1 = URS | CEN; // démarre timer
}

void Timer2_PWMInit(void)
{
    Timer2_Stop();

    TIM2_PSC = 79u; // tick de 10 us
    TIM2_ARR = 9u; // interruption chaque 100 us

    compteurPWM = 0u;
    modeTimer = TIMER2_MODE_PWM;

    if (PWM_DUTY_PERCENT > 0u)
    {
        LED_Set(LED_ON);
    }
    else
    {
        LED_Set(LED_OFF);
    }

    Timer2_Start(); // démarre PWM
}

void Timer2_InterruptInit(void)
{
    Timer2_Stop();

    TIM2_PSC = 7999u; // tick de 1 ms
    TIM2_ARR = 499u; // interruption chaque 500 ms

    modeTimer = TIMER2_MODE_INTERRUPT;

    Timer2_Start();
}

void TIM2_IRQHandler(void)
{
    if ((TIM2_SR & UIF) == 0u)
    {
        return;
    }

    TIM2_SR = 0u; // efface flag

    if (modeTimer == TIMER2_MODE_PWM)
    {
        if (compteurPWM < PWM_DUTY_PERCENT)
        {
            LED_Set(LED_ON);
        }
        else
        {
            LED_Set(LED_OFF);
        }

        compteurPWM++;

        if (compteurPWM >= PWM_STEPS)
        {
            compteurPWM = 0u;
        }
    }
    else if (modeTimer == TIMER2_MODE_INTERRUPT)
    {
        LED_Toggle(); // change LED à chaque interruption
    }
}