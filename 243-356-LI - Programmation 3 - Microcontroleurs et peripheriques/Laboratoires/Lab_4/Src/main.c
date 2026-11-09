#include "ADC.h"
#include "LCD.h"

#include <stdint.h>

#define RCC_CR   (*(volatile uint32_t *)0x40021000UL)
#define RCC_CFGR (*(volatile uint32_t *)0x40021004UL)
#define RCC_APB2ENR (*(volatile uint32_t *)0x40021018UL)

#define AFIO_MAPR (*(volatile uint32_t *)0x40010004UL)

#define GPIOB_CRL  (*(volatile uint32_t *)0x40010C00UL)
#define GPIOB_IDR  (*(volatile uint32_t *)0x40010C08UL)
#define GPIOB_BSRR (*(volatile uint32_t *)0x40010C10UL)

#define SYST_CVR (*(volatile uint32_t *)0xE000E018UL)

#define HSI_ON       (1UL << 0U)
#define HSI_READY    (1UL << 1U)

#define SYSCLK_SW    (3UL << 0U)
#define SYSCLK_SWS   (3UL << 2U)

#define AHB_PRE      (0xFUL << 4U)
#define APB1_PRE     (7UL << 8U)
#define APB2_PRE     (7UL << 11U)

#define AFIO_CLOCK_ENABLE (1UL << 0U)
#define GPIOB_CLOCK_ENABLE (1UL << 3U)
#define AFIO_SWJ_CFG_MASK (7UL << 24U)
#define AFIO_SWJ_NO_JTAG_KEEP_SWD (2UL << 24U)

#define BOUTON_ADC_PIN 3U  /* S1-4 */
#define BOUTON_MESSAGE_PIN 5U /* S1-3 */
#define BOUTON_ADC_MASK (1UL << BOUTON_ADC_PIN)
#define BOUTON_MESSAGE_MASK (1UL << BOUTON_MESSAGE_PIN)
#define GPIOB_BOUTONS_CONFIG_MASK ((0xFUL << 12U) | (0xFUL << 20U))
#define GPIOB_BOUTONS_PULLUP_CONFIG ((0x8UL << 12U) | (0x8UL << 20U))

#define SYSTICK_MASK 0x00FFFFFFUL
#define CPU_HZ 8000000UL
#define BOUTON_ANTIREBOND_MS 20UL
#define ADC1_CODE_MAX 4095UL
#define SERVO_ANGLE_MAX 180UL
#define SERVO_PWM_MIN 205UL
#define SERVO_PWM_MAX 410UL

static const uint8_t LCD_ADRESSE_LIGNE[4] = {0x00U, 0x40U, 0x14U, 0x54U};
static const char HEXADECIMAL[] = "0123456789ABCDEF";

static void LCD_Ecrire_Ligne(uint8_t ligne, const char *texte)
{
    uint8_t colonne;

    if ((ligne < 1U) || (ligne > 4U))
    {
        return;
    }

    LCD_Commande(false, false,
                 (uint8_t)(0x80U | LCD_ADRESSE_LIGNE[ligne - 1U]));

    for (colonne = 0U; colonne < 20U; colonne++)
    {
        if ((texte != 0) && (*texte != '\0'))
        {
            LCD_Commande(false, true, (uint8_t)*texte);
            texte++;
        }
        else
        {
            LCD_Commande(false, true, (uint8_t)' ');
        }
    }
}

static void Boutons_Init(void)
{
    RCC_APB2ENR |= AFIO_CLOCK_ENABLE | GPIOB_CLOCK_ENABLE;

    /* PB3 est JTDO: libère PB3/PB4 tout en conservant le débogage SWD. */
    AFIO_MAPR = (AFIO_MAPR & ~AFIO_SWJ_CFG_MASK)
              | AFIO_SWJ_NO_JTAG_KEEP_SWD;

    /* PB3 et PB5 en entrée avec pull-up; les boutons les mettent à 0. */
    GPIOB_CRL = (GPIOB_CRL & ~GPIOB_BOUTONS_CONFIG_MASK)
              | GPIOB_BOUTONS_PULLUP_CONFIG;
    GPIOB_BSRR = BOUTON_ADC_MASK | BOUTON_MESSAGE_MASK;
}

static bool Bouton_Est_Appuye(uint32_t masque)
{
    return (GPIOB_IDR & masque) == 0UL;
}

static void Delai_1ms(void)
{
    uint32_t depart = SYST_CVR;
    const uint32_t cycles = CPU_HZ / 1000UL;

    while (((depart - SYST_CVR) & SYSTICK_MASK) < cycles)
    {
    }
}

static void Delai_Ms(uint32_t millisecondes)
{
    while (millisecondes > 0UL)
    {
        Delai_1ms();
        millisecondes--;
    }
}

static bool Attendre_En_Verifiant_Bouton_ADC(uint32_t millisecondes)
{
    while (millisecondes > 0UL)
    {
        if (Bouton_Est_Appuye(BOUTON_ADC_MASK))
        {
            Delai_Ms(BOUTON_ANTIREBOND_MS);
            if (Bouton_Est_Appuye(BOUTON_ADC_MASK))
            {
                return false;
            }
        }

        Delai_1ms();
        millisecondes--;
    }

    return true;
}

static bool Afficher_Sequence_Portes_Ouvertes(void)
{
    LCD_Ecrire_Ligne(1U, "PORTES OUVERTES");
    LCD_Ecrire_Ligne(2U, "GENIE ELECTRIQUE");
    if (!Attendre_En_Verifiant_Bouton_ADC(3000UL))
    {
        return false;
    }

    LCD_Ecrire_Ligne(1U, "JE SUIS STM32");
    LCD_Ecrire_Ligne(2U, "UN MICROCONTROLEUR");
    if (!Attendre_En_Verifiant_Bouton_ADC(3000UL))
    {
        return false;
    }

    LCD_Ecrire_Ligne(1U, "ELECTRONIQUE");
    LCD_Ecrire_Ligne(2U, "+ PROGRAMMATION");
    if (!Attendre_En_Verifiant_Bouton_ADC(2000UL))
    {
        return false;
    }

    LCD_Ecrire_Ligne(1U, "APPUIE SUR S1-4");
    LCD_Ecrire_Ligne(2U, "POUR ME TESTER");
    return Attendre_En_Verifiant_Bouton_ADC(4000UL);
}

static void Effacer_LCD(void)
{
    uint8_t ligne;

    for (ligne = 1U; ligne <= 4U; ligne++)
    {
        LCD_Ecrire_Ligne(ligne, "");
    }
}

static uint16_t ADC_ConvertirEnAngle(uint16_t codeADC)
{
    return (uint16_t)((((uint32_t)codeADC * SERVO_ANGLE_MAX)
                     + (ADC1_CODE_MAX / 2UL)) / ADC1_CODE_MAX);
}

static void Afficher_Donnees_I2C(uint16_t angle)
{
    uint16_t valeurPWM;
    uint8_t valeurBasse;
    uint8_t valeurHaute;
    char ligne3[21] = "I2C:0x40 06=00";
    char ligne4[21] = "07=00 08=00 09=00";

    valeurPWM = (uint16_t)(SERVO_PWM_MIN
              + (((uint32_t)angle * (SERVO_PWM_MAX - SERVO_PWM_MIN))
              / SERVO_ANGLE_MAX));
    valeurBasse = (uint8_t)(valeurPWM & 0xFFU);
    valeurHaute = (uint8_t)(valeurPWM >> 8U);

    ligne4[9] = HEXADECIMAL[(valeurBasse >> 4U) & 0x0FU];
    ligne4[10] = HEXADECIMAL[valeurBasse & 0x0FU];
    ligne4[15] = HEXADECIMAL[(valeurHaute >> 4U) & 0x0FU];
    ligne4[16] = HEXADECIMAL[valeurHaute & 0x0FU];

    LCD_Ecrire_Ligne(3U, ligne3);
    LCD_Ecrire_Ligne(4U, ligne4);
}

static void Afficher_Mode_ADC(uint16_t codeADC)
{
    uint16_t angle = ADC_ConvertirEnAngle(codeADC);
    char ligneAngle[21] = "ANGLE: 000 deg";

    ligneAngle[7] = (char)('0' + ((angle / 100U) % 10U));
    ligneAngle[8] = (char)('0' + ((angle / 10U) % 10U));
    ligneAngle[9] = (char)('0' + (angle % 10U));

    LCD_Ecrire_Ligne(2U, ligneAngle);
    Afficher_Donnees_I2C(angle);
}

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
    Boutons_Init();

    while (1)
    {
        uint16_t codeADC;

        if (Bouton_Est_Appuye(BOUTON_MESSAGE_MASK))
        {
            Delai_Ms(BOUTON_ANTIREBOND_MS);
            if (Bouton_Est_Appuye(BOUTON_MESSAGE_MASK))
            {
                Effacer_LCD();
                (void)Afficher_Sequence_Portes_Ouvertes();
                Effacer_LCD();
                ADC1_ReinitialiserAffichage();

                while (Bouton_Est_Appuye(BOUTON_MESSAGE_MASK))
                {
                    Delai_1ms();
                }
            }
        }
        else if (Bouton_Est_Appuye(BOUTON_ADC_MASK))
        {
            Delai_Ms(BOUTON_ANTIREBOND_MS);
            if (Bouton_Est_Appuye(BOUTON_ADC_MASK))
            {
                Effacer_LCD();
                ADC1_ReinitialiserAffichage();
                while (Bouton_Est_Appuye(BOUTON_ADC_MASK))
                {
                    Delai_1ms();
                }
            }
        }

        codeADC = LCD_Affichage_Potentiometre();
        Afficher_Mode_ADC(codeADC);
    }
}
