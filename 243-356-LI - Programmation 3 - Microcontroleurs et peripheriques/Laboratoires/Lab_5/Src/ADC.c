#include "ADC.h"
#include "LCD.h"
#include "SysTick.h"
#include <stdbool.h>
#include <stdint.h>

#define RCC_CFGR   (*(volatile uint32_t *)0x40021004UL)
#define RCC_APB2ENR (*(volatile uint32_t *)0x40021018UL)

#define GPIOA_CRL (*(volatile uint32_t *)0x40010800UL)

#define ADC1_SR    (*(volatile uint32_t *)0x40012400UL)
#define ADC1_CR1   (*(volatile uint32_t *)0x40012404UL)
#define ADC1_CR2   (*(volatile uint32_t *)0x40012408UL)
#define ADC1_SMPR2 (*(volatile uint32_t *)0x40012410UL)
#define ADC1_SQR1  (*(volatile uint32_t *)0x4001242CUL)
#define ADC1_SQR3  (*(volatile uint32_t *)0x40012434UL)
#define ADC1_DR    (*(volatile uint32_t *)0x4001244CUL)

#define SYST_CVR (*(volatile uint32_t *)0xE000E018UL)

#define RCC_GPIOA_CLOCK_ENABLE (1UL << 2U)
#define RCC_ADC1_CLOCK_ENABLE  (1UL << 9U)
#define RCC_ADC_PRESCALER_MASK (3UL << 14U)
#define RCC_ADC_PRESCALER_DIV2 0UL

#define GPIO_PA0_CONFIG_MASK 0x0FUL

#define ADC1_EOC             (1UL << 1U)
#define ADC1_ADON            (1UL << 0U)
#define ADC1_RSTCAL          (1UL << 3U)
#define ADC1_CAL             (1UL << 2U)
#define ADC1_EXTSEL_SOFTWARE (7UL << 17U)
#define ADC1_EXTTRIG         (1UL << 20U)
#define ADC1_SWSTART         (1UL << 22U)
#define ADC1_RANK1_CHANNEL0  0UL

#define ADC1_CHANNEL0_SAMPLE_MASK  7UL
#define ADC1_CHANNEL0_SAMPLE_239_5 7UL
#define ADC1_SEQUENCE_LENGTH_MASK  (15UL << 20U)
#define ADC1_RANK1_CHANNEL_MASK    31UL
#define ADC1_DATA_MASK             0x0FFFUL

#define SYSTICK_MASK 0x00FFFFFFUL
#define CPU_HZ       8000000UL
#define ADC1_SETTLING_DELAY_US 2UL
#define CYCLES_PER_US (CPU_HZ / 1000000UL)
#define ADC1_SETTLING_CYCLES (ADC1_SETTLING_DELAY_US * CYCLES_PER_US)
#define ADC1_REFRESH_HZ 20UL
#define ADC1_REFRESH_CYCLES (CPU_HZ / ADC1_REFRESH_HZ)

#define LCD_FIRST_LINE 1U
#define LCD_FIRST_COLUMN 1U
#define LCD_TEXT_LENGTH 11U
#define ADC1_ROUNDING_OFFSET (ADC1_CODE_MAX / 2UL)


static bool affichageInitialise = false;
static uint32_t dernierAffichageCycle = 0UL;
static char ancienTexte[LCD_TEXT_LENGTH] = {0};

static void ADC1_AttendreCycles(uint32_t cycles)
{
    uint32_t depart = SYST_CVR;

    while (((depart - SYST_CVR) & SYSTICK_MASK) < cycles)
    {
    }
}

static void ADC1_AttendreRafraichissement(void) 
{
    static uint32_t dernierTempsMs = 0;
    uint32_t tempsActuel = SysTick_GetMs();
    
    if (tempsActuel - dernierTempsMs < 50) // Attendre 50 ms
    {
        SysTick_DelaiMs(50 - (tempsActuel - dernierTempsMs));
    }
    
    dernierTempsMs = SysTick_GetMs();
}

static uint32_t ADC1_ConvertirEnMillivolts(uint16_t code)
{
    uint32_t millivolts;

    millivolts = (((uint32_t)code * ADC1_VREF_MV) + ADC1_ROUNDING_OFFSET)
               / ADC1_CODE_MAX;



    return millivolts;
}

static void ADC1_FormaterTension(uint32_t millivolts, char texte[LCD_TEXT_LENGTH])
{
    texte[0] = 'A';
    texte[1] = 'D';
    texte[2] = 'C';
    texte[3] = '1';
    texte[4] = ':';
    texte[5] = (char)('0' + (millivolts / 1000UL));
    texte[6] = '.';
    texte[7] = (char)('0' + ((millivolts / 100UL) % 10UL));
    texte[8] = (char)('0' + ((millivolts / 10UL) % 10UL));
    texte[9] = (char)('0' + (millivolts % 10UL));
    texte[10] = 'V';
}

static void ADC1_EcrireTexte(const char texte[LCD_TEXT_LENGTH])
{
    uint8_t index;

    for (index = 0U; index < LCD_TEXT_LENGTH; index++)
    {
        if ((!affichageInitialise) || (texte[index] != ancienTexte[index]))
        {
            LCD_Ecrire_Char((unsigned char)texte[index],
                            LCD_FIRST_LINE,
                            (uint8_t)(LCD_FIRST_COLUMN + index));
            ancienTexte[index] = texte[index];
        }
    }

    affichageInitialise = true;
}

void ADC1_Init(void)
{
    /* SystemInit() fixe HSI et PCLK2 à 8 MHz; ADCPRE = 00 donne 4 MHz. */
    RCC_APB2ENR |= RCC_GPIOA_CLOCK_ENABLE | RCC_ADC1_CLOCK_ENABLE;
    RCC_CFGR = (RCC_CFGR & ~RCC_ADC_PRESCALER_MASK)
             | RCC_ADC_PRESCALER_DIV2;

    /* PA0: MODE0=00 et CNF0=00, donc entrée analogique. */
    GPIOA_CRL &= ~GPIO_PA0_CONFIG_MASK;

    ADC1_CR1 = 0UL; /* mode indépendant, une seule conversion régulière */
    ADC1_CR2 = ADC1_EXTSEL_SOFTWARE | ADC1_EXTTRIG;
    /* CONT=0, alignement à droite et déclenchement logiciel sélectionné. */
    ADC1_SMPR2 = (ADC1_SMPR2 & ~ADC1_CHANNEL0_SAMPLE_MASK)
               | ADC1_CHANNEL0_SAMPLE_239_5;
    ADC1_SQR1 &= ~ADC1_SEQUENCE_LENGTH_MASK; /* longueur L=0: une conversion */
    ADC1_SQR3 = (ADC1_SQR3 & ~ADC1_RANK1_CHANNEL_MASK)
              | ADC1_RANK1_CHANNEL0; /* rang 1: canal 0 */
    ADC1_SR = 0UL;

    ADC1_CR2 |= ADC1_ADON;
    ADC1_AttendreCycles(ADC1_SETTLING_CYCLES); /* 2 us après ADON; SysTick lancé par LCD_Init */

    ADC1_CR2 |= ADC1_RSTCAL;
    while ((ADC1_CR2 & ADC1_RSTCAL) != 0UL)
    {
    }

    ADC1_CR2 |= ADC1_CAL;
    while ((ADC1_CR2 & ADC1_CAL) != 0UL)
    {
    }
}

uint16_t ADC1_Lire(void)
{
    uint32_t resultat;

    (void)ADC1_DR; /* lecture de DR acquitte un éventuel EOC précédent */
    ADC1_CR2 |= ADC1_SWSTART;

    while ((ADC1_SR & ADC1_EOC) == 0UL)
    {
    }

    resultat = ADC1_DR; /* la lecture de DR efface EOC */
    return (uint16_t)(resultat & ADC1_DATA_MASK);
}

void LCD_Affichage_Potentiometre(void)
{
    uint16_t code;
    uint32_t millivolts;
    char texte[LCD_TEXT_LENGTH];

    /* Premier affichage immédiat, puis une mesure toutes les 50 ms. */
    if (affichageInitialise)
    {
        ADC1_AttendreRafraichissement();
    }

    code = ADC1_Lire();
    millivolts = ADC1_ConvertirEnMillivolts(code);
    ADC1_FormaterTension(millivolts, texte);
    ADC1_EcrireTexte(texte);
    dernierAffichageCycle = SYST_CVR;
}
