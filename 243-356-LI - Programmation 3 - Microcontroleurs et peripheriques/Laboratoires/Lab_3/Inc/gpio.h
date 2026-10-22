#ifndef GPIO_H
#define GPIO_H

#define LED_OFF 0u
#define LED_ON  1u

void LED_Init(void);
void LED_Set(unsigned char etat);
void LED_Toggle(void);

void Button_Init(void);
unsigned char Button_Read(void);

#endif