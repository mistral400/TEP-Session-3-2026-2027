#ifndef TIMER2_H
#define TIMER2_H

#include <stdint.h>

void Timer2_Init(void);

/* Retourne le nombre de secondes depuis minuit, entre 0 et 86399. */
uint32_t Timer2_GetSecondsSinceMidnight(void);

/* Sert à détecter chaque interruption Update dans la boucle principale. */
uint32_t Timer2_GetUpdateCount(void);

/* Permet de préparer le test du passage de 23:59:59 à 00:00:00. */
unsigned char Timer2_SetTime(uint8_t heures, uint8_t minutes, uint8_t secondes);

#endif
