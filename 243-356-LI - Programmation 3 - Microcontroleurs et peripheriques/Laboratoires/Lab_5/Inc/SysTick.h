#ifndef SYSTICK_H
#define SYSTICK_H

#include <stdint.h>

void SysTick_Init(void); // base de temps 1 ms
uint32_t SysTick_GetMs(void); // retourne le nombre de ms écoulées depuis le démarrage du microcontrôleur
void SysTick_DelaiUs(uint32_t microsecondes); // attend un délai en microsecondes
void SysTick_DelaiMs(uint32_t millisecondes); // attend un certain nombre de ms avant de continuer l'exécution du programme

#endif
