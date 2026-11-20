#include "ADC.h"
#include "I2C.h"
#include "LCD.h"
#include "PCA9685.h"
#include "SysTick.h"

#define CANAL_SERVO       0U
#define ANGLE_MAX_DEGRES  180UL
#define DELAI_BOUCLE_MS   50UL

int main(void)
{
    uint16_t valeurADC;
    uint8_t angle;
    uint8_t angle_precedent = 255U;

    SysTick_Init();
    LCD_Init();
    ADC1_Init();
    I2C1_Init();

    if (!PCA9685_Init())
    {
        LCD_Ecrire_Texte("ERREUR PCA9685", 1U, 1U);
        while (1)
        {
        }
    }

    while (1)
    {
        valeurADC = ADC1_Lire();
        // Estimation linéaire : les codes ADC aux butées ne sont pas mesurés.
        angle = (uint8_t)((((uint32_t)valeurADC * ANGLE_MAX_DEGRES) + (ADC1_CODE_MAX / 2UL))
                        / ADC1_CODE_MAX);

        if (angle != angle_precedent)
        {
            char ligneAngle[21] = "ANGLE:000";

            ligneAngle[6] = (char)('0' + ((angle / 100U) % 10U));
            ligneAngle[7] = (char)('0' + ((angle / 10U) % 10U));
            ligneAngle[8] = (char)('0' + (angle % 10U));

            LCD_Ecrire_Texte(ligneAngle, 1U, 1U);

            if (PCA9685_Definir_Angle(CANAL_SERVO, angle))
            {
                angle_precedent = angle;
            }
        }

        SysTick_DelaiMs(DELAI_BOUCLE_MS);
    }
}
