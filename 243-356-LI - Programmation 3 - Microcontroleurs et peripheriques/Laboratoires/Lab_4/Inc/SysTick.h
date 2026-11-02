#ifndef SYSTICK_H
#define SYSTICK_H

void SysTick_Init(void); // base de temps 1 ms
unsigned int SysTick_GetMs(void); // retourne le nombre de ms écoulées depuis le démarrage du microcontrôleur

#endif  