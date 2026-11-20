#include "PCA9685.h"
#include "I2C.h"
#include "SysTick.h"

#define PCA9685_ADRESSE              0x40U

#define PCA9685_REG_MODE1            0x00U
#define PCA9685_REG_LED0_ON_L        0x06U
#define PCA9685_REG_LED0_ON_H        0x07U
#define PCA9685_REG_LED0_OFF_L       0x08U
#define PCA9685_REG_LED0_OFF_H       0x09U
#define PCA9685_REG_PRESCALE         0xFEU

#define PCA9685_MODE1_SLEEP          0x10U
#define PCA9685_MODE1_AUTO_INCREMENT 0x20U
#define PCA9685_MODE1_RESTART        0x80U
#define PCA9685_MODE1_NORMAL         0x00U

#define PCA9685_PRESCALE_50_HZ       121U
#define PCA9685_NOMBRE_CANAUX        16U
#define PCA9685_REGISTRES_PAR_CANAL  4U
#define PCA9685_COMPTE_MAX           4095U

#define PCA9685_SERVO_MIN            205U // 1,0 ms à 50 Hz (205 / 4096 * 20 ms = 1,001 ms)
#define PCA9685_SERVO_MAX            410U // 2,0 ms à 50 Hz (410 / 4096 * 20 ms = 2,002 ms)
#define PCA9685_SERVO_ANGLE_MAX      180U

#define PCA9685_OCTET_MASQUE         0xFFU
#define PCA9685_OCTET_DECALAGE       8U

// Initialise le PCA9685 pour fonctionner à 50 Hz en mode normal
bool PCA9685_Init(void)
{
    // 1. Mettre le circuit en veille (SLEEP) pour pouvoir modifier le PRESCALE
    if (!I2C1_Ecrire_Registre(PCA9685_ADRESSE, PCA9685_REG_MODE1, PCA9685_MODE1_SLEEP))
    {
        return false;
    }

    // 2. Configurer la fréquence PWM à 50 Hz (PRESCALE = 121 pour horloge interne 25 MHz)
    if (!I2C1_Ecrire_Registre(PCA9685_ADRESSE, PCA9685_REG_PRESCALE, PCA9685_PRESCALE_50_HZ))
    {
        return false;
    }

    // 3. Sortir du mode SLEEP avec auto-incrémentation activée
    if (!I2C1_Ecrire_Registre(PCA9685_ADRESSE, PCA9685_REG_MODE1, PCA9685_MODE1_AUTO_INCREMENT))
    {
        return false;
    }

    // 4. Délai d'au moins 500 µs pour la stabilisation de l'oscillateur interne
    SysTick_DelaiMs(1UL);

    // 5. Réveiller et redémarrer la logique PWM (RESTART)
    if (!I2C1_Ecrire_Registre(PCA9685_ADRESSE, PCA9685_REG_MODE1,
                              PCA9685_MODE1_RESTART | PCA9685_MODE1_AUTO_INCREMENT))
    {
        return false;
    }

    return true;
}

// Configure les comptes de début (ON) et de fin (OFF) pour un canal PWM (0 à 15)
bool PCA9685_Definir_PWM(uint8_t canal, uint16_t debut, uint16_t fin)
{
    uint8_t registreBase;

    if ((canal >= PCA9685_NOMBRE_CANAUX)
        || (debut > PCA9685_COMPTE_MAX)
        || (fin > PCA9685_COMPTE_MAX))
    {
        return false;
    }

    registreBase = PCA9685_REG_LED0_ON_L + (canal * PCA9685_REGISTRES_PAR_CANAL);

    if (!I2C1_Ecrire_Registre(PCA9685_ADRESSE, registreBase,
                              (uint8_t)(debut & PCA9685_OCTET_MASQUE)))
    {
        return false;
    }

    if (!I2C1_Ecrire_Registre(PCA9685_ADRESSE, (uint8_t)(registreBase + 1U),
                              (uint8_t)((debut >> PCA9685_OCTET_DECALAGE) & PCA9685_OCTET_MASQUE)))
    {
        return false;
    }

    if (!I2C1_Ecrire_Registre(PCA9685_ADRESSE, (uint8_t)(registreBase + 2U),
                              (uint8_t)(fin & PCA9685_OCTET_MASQUE)))
    {
        return false;
    }

    if (!I2C1_Ecrire_Registre(PCA9685_ADRESSE, (uint8_t)(registreBase + 3U),
                              (uint8_t)((fin >> PCA9685_OCTET_DECALAGE) & PCA9685_OCTET_MASQUE)))
    {
        return false;
    }

    return true;
}


bool PCA9685_Definir_Angle(uint8_t canal, uint8_t angle)
{
    uint16_t compteFin;

    if ((canal >= PCA9685_NOMBRE_CANAUX) || (angle > PCA9685_SERVO_ANGLE_MAX))
    {
        return false;
    }

    compteFin = (uint16_t)(PCA9685_SERVO_MIN
              + (((uint32_t)angle * (PCA9685_SERVO_MAX - PCA9685_SERVO_MIN))
                 / PCA9685_SERVO_ANGLE_MAX));

    return PCA9685_Definir_PWM(canal, 0U, compteFin);
}
