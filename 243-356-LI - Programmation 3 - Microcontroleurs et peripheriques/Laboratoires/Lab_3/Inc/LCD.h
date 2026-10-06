#ifndef LCD_H
#define LCD_H

#include <stdbool.h>
#include <stdint.h>

void LCD_Init(void);
void LCD_Commande(bool RW, bool RS, uint8_t Data);
void LCD_Ecrire_Char(unsigned char caractere, uint8_t ligne, uint8_t colonne);
void LCD_Affichage_Temps(void);

#endif
