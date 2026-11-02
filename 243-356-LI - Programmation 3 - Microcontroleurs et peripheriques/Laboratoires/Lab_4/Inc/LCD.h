#ifndef LCD_H
#define LCD_H

#include <stdbool.h>
#include <stdint.h>

void LCD_Init(void); // Configure le LCD pour l'affichage
void LCD_Commande(bool RW, bool RS, uint8_t Data); // Envoie une commande ou des données au LCD
void LCD_Ecrire_Char(unsigned char caractere, uint8_t ligne, uint8_t colonne); // Écrit un caractère à la position spécifiée sur le LCD

#endif
