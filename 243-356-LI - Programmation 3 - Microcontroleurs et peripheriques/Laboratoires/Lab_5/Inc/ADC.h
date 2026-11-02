#ifndef ADC_H
#define ADC_H

#include <stdint.h>

// Valeur maximale de l'ADC1 que j'ai trouver sur HAL
#define ADC1_CODE_MAX 4095UL

// Valeur de référence de l'ADC1 en mV que j'ai trouver sur HAL
#define ADC1_VREF_MV 3300UL
// Je vais devoir les remplacé lorsque je vais faire les mesures
#define ADC1_VMIN_MV 0UL
#define ADC1_VMAX_MV 0UL
#define ADC1_LIMITES_MESUREES 0U

void ADC1_Init(void); // Configure l'ADC1 pour lire la valeur du potentiomètre
uint16_t ADC1_Lire(void); // Lit la valeur de l'ADC1 et retourne la valeur ADC 4095
void LCD_Affichage_Potentiometre(void); // Affiche en mv l'ADC sur le LCD

#endif
