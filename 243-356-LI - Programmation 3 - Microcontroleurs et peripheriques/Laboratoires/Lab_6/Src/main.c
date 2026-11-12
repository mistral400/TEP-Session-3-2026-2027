#include "ADC.h"
#include "I2C.h"
#include "LCD.h"
#include "PCA9685.h"
#include "SysTick.h"
#include "gpio.h"

#define SERVO_ANGLE_MAX 180UL
#define SERVO_NOMBRE 3U

static void LCD_Afficher_Ligne_Moteur(uint8_t moteur,
                                      uint8_t angle,
                                      uint8_t moteurSelectionne)
{
    char ligne[21] = "                    ";

    ligne[0] = 'M';
    ligne[1] = (char)('1' + moteur);
    ligne[2] = ':';
    ligne[3] = (char)('0' + ((angle / 100U) % 10U));
    ligne[4] = (char)('0' + ((angle / 10U) % 10U));
    ligne[5] = (char)('0' + (angle % 10U));

    if (moteur == moteurSelectionne)
    {
        ligne[15] = '<';
        ligne[16] = '<';
        ligne[17] = '<';
        ligne[18] = '<';
        ligne[19] = '<';
    }

    LCD_Ecrire_Texte(ligne, (uint8_t)(moteur + 1U), 1U);
}

static void LCD_Afficher_Angles(const uint8_t angles[SERVO_NOMBRE],
                                uint8_t moteurSelectionne)
{
    uint8_t moteur;

    for (moteur = 0U; moteur < SERVO_NOMBRE; moteur++)
    {
        LCD_Afficher_Ligne_Moteur(moteur,
                                  angles[moteur],
                                  moteurSelectionne);
    }
}

int main(void)
{
    uint16_t valeurADC;
    uint8_t anglePotentiometre;
    uint8_t anglesMoteurs[SERVO_NOMBRE] = {0U, 0U, 0U};
    uint8_t moteurSelectionne = 0U;
    uint8_t bouton3;
    uint8_t affichageNecessaire;
    uint8_t index;

    SysTick_Init();
    LCD_Init();
    Button_Init();
    ADC1_Init();
    I2C_Init();

    PCA9685_ConfigurerPWM0();

    for (index = 0U; index < SERVO_NOMBRE; index++)
    {
        PCA9685_SetServoAngle(index, anglesMoteurs[index]);
    }

    LCD_Afficher_Angles(anglesMoteurs, moteurSelectionne);

    while (1)
    {
        affichageNecessaire = 0U;

        /* Chaque appui sur le bouton 3 passe au moteur suivant. */
        bouton3 = Button_Read();
        if (bouton3 != 0U)
        {
            moteurSelectionne++;
            if (moteurSelectionne >= SERVO_NOMBRE)
            {
                moteurSelectionne = 0U;
            }
            affichageNecessaire = 1U;
        }

        valeurADC = ADC1_Lire();
        anglePotentiometre = (uint8_t)((((uint32_t)valeurADC * SERVO_ANGLE_MAX)
                                      + (ADC1_CODE_MAX / 2UL))
                                     / ADC1_CODE_MAX);

        if (anglePotentiometre != anglesMoteurs[moteurSelectionne])
        {
            anglesMoteurs[moteurSelectionne] = anglePotentiometre;
            PCA9685_SetServoAngle(moteurSelectionne, anglePotentiometre);
            affichageNecessaire = 1U;
        }

        if (affichageNecessaire != 0U)
        {
            LCD_Afficher_Angles(anglesMoteurs, moteurSelectionne);
        }

        SysTick_DelaiMs(50UL); // Délai court pour une réponse plus fluide
    }
}
