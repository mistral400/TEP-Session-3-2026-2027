#ifndef GPIO_H
#define GPIO_H

#define LED_OFF 0u // Met l'état de la LED à éteinte
#define LED_ON  1u // Met l'état de la LED à allumée

void LED_Init(void); // Configure la broche de la LED comme sortie
void LED_Set(unsigned char etat); // Met l'état de la LED (LED_ON ou LED_OFF)
void LED_Toggle(void); // Change l'état de la LED

void Button_Init(void); // Configure la broche du bouton comme entrée avec résistance de tirage interne activée
unsigned char Button_Read(void); // Lit l'état du bouton 

#endif