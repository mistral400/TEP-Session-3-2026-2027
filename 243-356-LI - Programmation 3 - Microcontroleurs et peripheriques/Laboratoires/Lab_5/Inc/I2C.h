#ifndef I2C_H
#define I2C_H

#include <stdbool.h>
#include <stdint.h>

#define I2C_OK                  0U
#define I2C_ERREUR_TIMEOUT      1U
#define I2C_ERREUR_NACK         2U
#define I2C_ERREUR_BERR         3U
#define I2C_ERREUR_ARLO         4U
#define I2C_ERREUR_BUS_OCCUPE   5U
#define I2C_ERREUR_ADRESSE      6U
#define I2C_ERREUR_SEQUENCE     7U

void I2C1_Init(void);
bool I2C1_Ecrire_Registre(uint8_t adresse, uint8_t registre, uint8_t donnee);
bool I2C1_Lire_Registre(uint8_t adresse, uint8_t registre, uint8_t *donnee);

#endif
