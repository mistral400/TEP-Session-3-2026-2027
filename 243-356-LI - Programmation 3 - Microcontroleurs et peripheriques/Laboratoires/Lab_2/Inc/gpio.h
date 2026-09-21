#ifndef GPIO_H
#define GPIO_H

#define LED_OFF 0u
#define LED_ON  1u

#define LED_ACTIVE_LOW 1u
#define BUTTON_ACTIVE_LOW 1u
#define BUTTON_INTERNAL_PULL 0u // 0 flottante, 1 pull-up, 2 pull-down

void LED_Init(void);
void LED_Set(unsigned char etat);
void LED_Toggle(void);

void Button_Init(void);
unsigned char Button_Read(void);

#endif