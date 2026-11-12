#include "ADC.h"
#include "I2C.h"
#include "LCD.h"
#include "PCA9685.h"
#include "SysTick.h"

int main(void)
{
    uint16_t valeurADC;
    uint8_t angle;
    uint8_t angle_precedent = 255U;

    SysTick_Init();
    LCD_Init();
    ADC1_Init();
    I2C_Init();


    PCA9685_ConfigurerPWM0();

    while (1)
    {
        valeurADC = ADC1_Lire();
        angle = (uint8_t)((((uint32_t)valeurADC * 180UL) + 2047UL)
                        / ADC1_CODE_MAX);

        if (angle != angle_precedent)
        {
            char ligneAngle[21] = "ANGLE:000";

            ligneAngle[6] = (char)('0' + ((angle / 100U) % 10U));
            ligneAngle[7] = (char)('0' + ((angle / 10U) % 10U));
            ligneAngle[8] = (char)('0' + (angle % 10U));

            LCD_Ecrire_Texte(ligneAngle, 1U, 1U);
            angle_precedent = angle;
        }

        PCA9685_SetServoAngle(0U, angle);
        SysTick_DelaiMs(50UL); // Pour éviter de saturer
    }
}
