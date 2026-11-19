#include "ADC.h"
#include "gpio.h"
#include "SysTick.h"

#include <stdint.h>

#define RCC_CFGR    (*(volatile uint32_t *)0x40021004UL)
#define RCC_APB2ENR (*(volatile uint32_t *)0x40021018UL)

#define ADC1_SR    (*(volatile uint32_t *)0x40012400UL)
#define ADC1_CR1   (*(volatile uint32_t *)0x40012404UL)
#define ADC1_CR2   (*(volatile uint32_t *)0x40012408UL)
#define ADC1_SMPR2 (*(volatile uint32_t *)0x40012410UL)
#define ADC1_SQR1  (*(volatile uint32_t *)0x4001242CUL)
#define ADC1_SQR3  (*(volatile uint32_t *)0x40012434UL)
#define ADC1_DR    (*(volatile uint32_t *)0x4001244CUL)

#define RCC_ADC1_CLOCK_ENABLE  (1UL << 9U)
#define RCC_ADC_PRESCALER_MASK (3UL << 14U)
#define RCC_ADC_PRESCALER_DIV2 0UL

#define ADC1_SETTLING_DELAY_US 2UL
#define ADC1_EOC                (1UL << 1U)
#define ADC1_ADON               (1UL << 0U)
#define ADC1_RSTCAL             (1UL << 3U)
#define ADC1_CAL                (1UL << 2U)
#define ADC1_EXTSEL_SOFTWARE    (7UL << 17U)
#define ADC1_EXTTRIG            (1UL << 20U)
#define ADC1_SWSTART            (1UL << 22U)
#define ADC1_RANK1_CHANNEL0     0UL

#define ADC1_CHANNEL0_SAMPLE_MASK  7UL
#define ADC1_CHANNEL0_SAMPLE_239_5 7UL
#define ADC1_SEQUENCE_LENGTH_MASK  (15UL << 20U)
#define ADC1_RANK1_CHANNEL_MASK    31UL
#define ADC1_DATA_MASK             0x0FFFUL

void ADC1_Init(void)
{// Configure l'ADC1 pour lire la valeur du potentiomètre
    RCC_APB2ENR |= RCC_ADC1_CLOCK_ENABLE;
    GPIOA_ConfigurerPA0Analogique(); // Configure PA0 comme entrée analogique

    RCC_CFGR = (RCC_CFGR & ~RCC_ADC_PRESCALER_MASK)
             | RCC_ADC_PRESCALER_DIV2;

    ADC1_CR1 = 0UL; // pas d'interruption, pas de mode scan
    ADC1_CR2 = ADC1_EXTSEL_SOFTWARE | ADC1_EXTTRIG;
    // Configure l'ADC1 pour lire le canal 0 (PA0)
    ADC1_SMPR2 = (ADC1_SMPR2 & ~ADC1_CHANNEL0_SAMPLE_MASK)
               | ADC1_CHANNEL0_SAMPLE_239_5;
    ADC1_SQR1 &= ~ADC1_SEQUENCE_LENGTH_MASK; // 1 conversion par séquence
    ADC1_SQR3 = (ADC1_SQR3 & ~ADC1_RANK1_CHANNEL_MASK)
              | ADC1_RANK1_CHANNEL0; // canal 0 en rang 1
    ADC1_SR = 0UL; // pas d'interruption, pas de drapeau EOC

    ADC1_CR2 |= ADC1_ADON; // allume l'ADC1
    SysTick_DelaiUs(ADC1_SETTLING_DELAY_US); // temps de stabilisation de l'ADC1

    ADC1_CR2 |= ADC1_RSTCAL; // réinitialise la calibration de l'ADC1
    while ((ADC1_CR2 & ADC1_RSTCAL) != 0UL) // attendre la fin de la calibration
    {
    }

    ADC1_CR2 |= ADC1_CAL; // lance la calibration de l'ADC1
    while ((ADC1_CR2 & ADC1_CAL) != 0UL) // attendre la fin de la calibration
    {
    }
}

uint16_t ADC1_Lire(void)
{
    uint32_t resultat;

    (void)ADC1_DR; // lecture de DR acquitte un éventuel EOC précédent
    ADC1_CR2 |= ADC1_SWSTART; // démarre la conversion de l'ADC1

    while ((ADC1_SR & ADC1_EOC) == 0UL) // attendre la fin de la conversion
    {
    }

    resultat = ADC1_DR; // lit la valeur de l'ADC1
    return (uint16_t)(resultat & ADC1_DATA_MASK); // retourne la valeur de l'ADC1
}
