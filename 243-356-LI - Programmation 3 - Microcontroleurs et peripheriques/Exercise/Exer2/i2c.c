#include "i2c.h"

typedef struct
{
    volatile uint32_t CR;
    volatile uint32_t SR;
    volatile uint32_t DR;
} I2C_TypeDef;

static I2C_TypeDef i2cSimule = {0};

#define I2C1 (&i2cSimule)

void i2c_init(void)
{
    I2C1->CR |= (1 << 0);
}

uint32_t i2c_lire_registre(uint8_t adresse)
{
    I2C1->CR = (I2C1->CR & ~0xFF00) | (adresse << 8);
    I2C1->CR |= (1 << 1);

    return 101325;
}