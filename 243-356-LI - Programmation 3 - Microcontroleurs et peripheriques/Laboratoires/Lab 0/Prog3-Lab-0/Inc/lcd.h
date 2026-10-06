#ifndef LCD_H
#define LCD_H

#include <stdbool.h>
#include <stdint.h>

/* Initialisation et commandes générales. */
void LCD_Init(void);
void LCD_Clear(void);
// !! CHANGEMENT : nouvelles commandes publiques de contrôle de l'afficheur.
void LCD_Home(void);
void LCD_DisplayOn(void);
void LCD_DisplayOff(void);
void LCD_SetCursorVisible(bool enabled);
void LCD_SetBlink(bool enabled);
void LCD_ShiftLeft(void);
void LCD_ShiftRight(void);

/* Écriture de texte. Les coordonnées commencent à zéro. */
// !! CHANGEMENT : nouvelle API de texte utilisant des chaînes const.
void LCD_Write(const char *text);
void LCD_WriteAt(uint8_t col, uint8_t row, const char *text);
void LCD_WriteLine(uint8_t row, const char *text);
void LCD_ClearLine(uint8_t row);
void LCD_CenterText(uint8_t row, const char *text);

/* Écriture de nombres, sans dépendance à printf. */
// !! CHANGEMENT : nouvelles conversions numériques légères.
void LCD_PrintInt(int32_t value);
void LCD_PrintUInt(uint32_t value);
void LCD_PrintHex(uint32_t value);
void LCD_PrintBinary(uint32_t value);
void LCD_PrintIntAt(uint8_t col, uint8_t row, int32_t value);

/* Caractères personnalisés et barre de progression. */
// !! CHANGEMENT : nouvelle API CGRAM et barre de progression.
void LCD_CreateChar(uint8_t slot, const uint8_t pattern[8]);
void LCD_PrintCustomChar(uint8_t slot);
void LCD_ProgressBar(uint8_t row, uint8_t value, uint8_t max);

/*
 * API historique conservée pour ne pas casser le code existant.
 * Pour le nouveau code, préférer les fonctions de plus haut niveau ci-dessus.
 */
void LCD_SendCommand(uint8_t cmd);
void LCD_SendChar(char data);
// !! CHANGEMENT : chaîne historique désormais const, sans rupture d'ABI.
void LCD_SendString(const char *str);
void LCD_SetCursor(uint8_t col, uint8_t row);

#endif /* LCD_H */
