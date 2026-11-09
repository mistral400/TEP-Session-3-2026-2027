#ifndef ADC_H
#define ADC_H

#include <stdint.h>

// Valeur maximale de l'ADC 12 bits
#define ADC1_CODE_MAX 4095UL

// Tension maximal e de référence de l'ADC1 en mV
#define ADC1_VREF_MV 3300UL

// Vmin mesurée au multimètre sur le pot: 0,0003 V
#define ADC1_VMIN_MV 0UL

// Vmax mesurée au multimètre sur le pot: 3,1964 V
#define ADC1_VMAX_MV 3196UL // vmax 


#define ADC1_LIMITES_MESUREES 1U // Actives

void ADC1_Init(void); // Configure l'ADC1 pour lire la valeur du potentiomètre
uint16_t ADC1_Lire(void); // Lit la valeur de l'ADC1 et retourne la valeur ADC 4095
uint16_t LCD_Affichage_Potentiometre(void); // Affiche la tension et retourne la mesure ADC
void ADC1_ReinitialiserAffichage(void); // Force le prochain affichage complet de l'ADC

#endif
