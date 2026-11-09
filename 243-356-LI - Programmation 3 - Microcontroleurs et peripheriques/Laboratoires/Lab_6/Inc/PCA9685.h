#ifndef PCA9685_H
#define PCA9685_H

#include "I2C.h"

uint8_t PCA9685_TesterCommunication(void);
uint8_t PCA9685_ConfigurerPWM0(void);
void PCA9685_SetDutyCycle(uint8_t canal, uint8_t dutyCycle);
void PCA9685_SetServoAngle(uint8_t canal, uint8_t angle);

#endif
