#ifndef GPIO_H
#define GPIO_H

#define LED_OFF 0u
#define LED_ON  1u

#define LED_ACTIVE_LOW 1u
#define BUTTON_ACTIVE_LOW 1u

// Modes de pull interne pour bouton
#define BUTTON_PULL_NONE   0u // pas de résistance de pull
#define BUTTON_PULL_UP     1u // pull-up interne
#define BUTTON_PULL_DOWN   2u // pull-down interne

#define BUTTON_INTERNAL_PULL BUTTON_PULL_NONE // pull-up interne (1) ou pull-down interne (2) ou pas de pull interne (0)

void LED_Init(void);
void LED_Set(unsigned char etat);
void LED_Toggle(void);

void Button_Init(void);
unsigned char Button_Read(void);

#endif