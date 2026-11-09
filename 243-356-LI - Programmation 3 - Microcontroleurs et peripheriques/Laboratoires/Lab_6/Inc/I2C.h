#ifndef I2C_H
#define I2C_H

#include <stdint.h>

#define I2C_OK                  0U
#define I2C_ERREUR_TIMEOUT      1U
#define I2C_ERREUR_NACK         2U
#define I2C_ERREUR_BERR         3U
#define I2C_ERREUR_ARLO         4U
#define I2C_ERREUR_BUS_OCCUPE   5U
#define I2C_ERREUR_ADRESSE      6U
#define I2C_ERREUR_SEQUENCE     7U

void I2C_Init(void);
uint8_t I2C_Start(void);
uint8_t I2C_AdresseEcriture(uint8_t adresse);
void I2C_EffacerADDR(void);
uint8_t I2C_EnvoyerDonnee(uint8_t donnee);
uint8_t I2C_Stop(void);
uint8_t I2C_EcrireOctet(uint8_t adresse, uint8_t donnee);

#endif
