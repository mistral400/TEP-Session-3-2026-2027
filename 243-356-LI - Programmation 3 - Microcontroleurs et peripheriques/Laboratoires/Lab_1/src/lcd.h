#ifndef LCD_H
#define LCD_H

#include <stdint.h>
#include <stdbool.h>

// ============================================================
// Initialisation et contrôle général
// ============================================================

// Initialise le LCD.
void LCD_Init(void);

// Efface complètement l'écran.
void LCD_Clear(void);

// Replace le curseur en haut à gauche sans effacer l'écran.
void LCD_Home(void);

// Active l'affichage.
void LCD_DisplayOn(void);

// Désactive l'affichage sans effacer le contenu.
void LCD_DisplayOff(void);


// ============================================================
// Curseur et affichage
// ============================================================

// Place le curseur à une position donnée.
void LCD_SetCursor(uint8_t col, uint8_t row);

// Affiche ou masque le curseur.
void LCD_SetCursorVisible(bool enabled);

// Active ou désactive le clignotement du curseur.
void LCD_SetBlink(bool enabled);

// Décale l'affichage d'une position vers la gauche.
void LCD_ShiftLeft(void);

// Décale l'affichage d'une position vers la droite.
void LCD_ShiftRight(void);


// ============================================================
// Fonctions d'écriture simples
// ============================================================

// Écrit du texte à partir de la position actuelle.
void LCD_Write(const char *text);

// Écrit du texte à une position précise.
void LCD_WriteAt(uint8_t col, uint8_t row, const char *text);

// Écrit une ligne complète.
// Le reste de la ligne est rempli avec des espaces.
void LCD_WriteLine(uint8_t row, const char *text);

// Efface seulement une ligne.
void LCD_ClearLine(uint8_t row);

// Centre automatiquement un texte sur une ligne.
void LCD_CenterText(uint8_t row, const char *text);


// ============================================================
// Nombres
// ============================================================

// Affiche un entier signé en décimal.
void LCD_PrintInt(int32_t value);

// Affiche un entier non signé en décimal.
void LCD_PrintUInt(uint32_t value);

// Affiche une valeur en hexadécimal.
void LCD_PrintHex(uint32_t value);

// Affiche une valeur en binaire.
void LCD_PrintBinary(uint32_t value);

// Affiche un entier signé à une position précise.
void LCD_PrintIntAt(uint8_t col, uint8_t row, int32_t value);


// ============================================================
// Caractères personnalisés
// ============================================================

// Crée un caractère personnalisé dans un emplacement CGRAM 0 à 7.
void LCD_CreateChar(uint8_t slot, const uint8_t pattern[8]);

// Affiche un caractère personnalisé précédemment créé.
void LCD_PrintCustomChar(uint8_t slot);


// ============================================================
// Widgets simples
// ============================================================

// Affiche une barre de progression sur une ligne complète.
void LCD_ProgressBar(uint8_t row, uint8_t value, uint8_t max);


// ============================================================
// API bas niveau / historique
// ============================================================

// Envoie directement une commande au contrôleur LCD.
void LCD_SendCommand(uint8_t cmd);

// Affiche un caractère.
void LCD_SendChar(char data);

// Affiche une chaîne de caractères.
void LCD_SendString(const char *str);


#endif // LCD_H