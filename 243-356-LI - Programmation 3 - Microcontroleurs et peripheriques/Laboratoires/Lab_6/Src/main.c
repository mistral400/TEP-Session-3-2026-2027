#include "I2C.h"
#include "LCD.h"
#include "PCA9685.h"
#include "SysTick.h"

int main(void)
{
    SysTick_Init();
    LCD_Init();
    I2C_Init();

    LCD_Ecrire_Texte("Allo c'est moi", 1U, 1U);
    LCD_Ecrire_Texte("Matisse", 2U, 1U);

    PCA9685_ConfigurerPWM0();

    while (1)
    {
        if (PCA9685_ConfigurerPWM0() == I2C_OK)
{
    LCD_Ecrire_Texte("PCA9685 OK", 3U, 1U);
}
else
{
    LCD_Ecrire_Texte("PCA9685 ABSENT", 3U, 1U);
}
        PCA9685_SetServoAngle(0U, 90U);
        SysTick_DelaiMs(1000UL);

        PCA9685_SetServoAngle(0U, 180U);
        SysTick_DelaiMs(1000UL);

        PCA9685_SetServoAngle(0U, 90U);
        SysTick_DelaiMs(1000UL);

        PCA9685_SetServoAngle(0U, 0U);
        SysTick_DelaiMs(1000UL);
    }
}