#ifndef I2C_H
#define I2C_H

#include <stdint.h>

void i2c_init(void);
uint32_t i2c_lire_registre(uint8_t adresse);

#endif