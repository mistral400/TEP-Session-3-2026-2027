#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

#define LED_OFF 0u // Met l'état de la LED à éteinte
#define LED_ON  1u // Met l'état de la LED à allumée

void LED_Init(void); // Configure la broche de la LED comme sortie
void LED_Set(uint8_t etat); // Met l'état de la LED (LED_ON ou LED_OFF)
void LED_Toggle(void); // Change l'état de la LED

void Button_Init(void); // Configure le bouton 3 sur PB5 avec pull-up
uint8_t Button_Read(void); // Retourne 1 une fois par appui debouncé

#endif
