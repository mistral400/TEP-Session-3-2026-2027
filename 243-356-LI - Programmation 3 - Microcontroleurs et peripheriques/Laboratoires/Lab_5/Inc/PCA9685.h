#ifndef PCA9685_H
#define PCA9685_H

#include <stdbool.h>
#include <stdint.h>

bool PCA9685_Init(void);
bool PCA9685_Definir_PWM(uint8_t canal, uint16_t debut, uint16_t fin);
bool PCA9685_Definir_Angle(uint8_t canal, uint8_t angle);

#endif
