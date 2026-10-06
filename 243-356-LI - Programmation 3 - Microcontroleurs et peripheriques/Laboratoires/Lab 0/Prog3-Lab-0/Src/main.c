#include "lcd.h"

int main(void)
{
    static const uint8_t degree_symbol[8] = {
        0x06U,
        0x09U,
        0x09U,
        0x06U,
        0x00U,
        0x00U,
        0x00U,
        0x00U
    };

    LCD_Init();

    /* Exemple principal demandé : texte et nombre positionnés simplement. */
    LCD_WriteLine(0U, "System Ready");
    LCD_WriteAt(0U, 1U, "Temp:");
    LCD_PrintIntAt(6U, 1U, 23);

    /* Test d'un caractère personnalisé : 23° C. */
    LCD_CreateChar(0U, degree_symbol);
    LCD_SetCursor(8U, 1U);
    LCD_PrintCustomChar(0U);
    LCD_Write("C");

    /* Test des conversions et de la barre de progression. */
    LCD_WriteAt(0U, 2U, "Hex:");
    LCD_SetCursor(5U, 2U);
    LCD_PrintHex(0x2AU);
    LCD_WriteAt(9U, 2U, "Bin:");
    LCD_SetCursor(13U, 2U);
    LCD_PrintBinary(5U);
    LCD_ProgressBar(3U, 65U, 100U);

    while (1)
    {
        /* Boucle principale de l'application. */
    }
}
