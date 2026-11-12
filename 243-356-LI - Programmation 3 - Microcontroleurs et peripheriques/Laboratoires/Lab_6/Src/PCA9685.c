#include "PCA9685.h"
#include "SysTick.h"

#define PCA9685_ADRESSE 0x40U

#define PCA9685_REG_MODE1       0x00U
#define PCA9685_REG_LED0_ON_L   0x06U
#define PCA9685_REG_LED0_ON_H   0x07U
#define PCA9685_REG_LED0_OFF_L  0x08U
#define PCA9685_REG_LED0_OFF_H  0x09U
#define PCA9685_REG_PRESCALE    0xFEU

#define PCA9685_MODE1_SLEEP          0x10U
#define PCA9685_MODE1_AUTO_INCREMENT 0x20U
#define PCA9685_MODE1_RESTART        0x80U

#define PCA9685_PRESCALE_50_HZ  121U
#define PCA9685_PWM0_ON_L       0x00U
#define PCA9685_PWM0_ON_H       0x00U
#define PCA9685_PWM0_OFF_L      0x33U
#define PCA9685_PWM0_OFF_H      0x01U
// modifie le duty cycle
#define PCA9685_NOMBRE_CANAUX 16U
#define PCA9685_PWM_MAX 4096UL
#define PCA9685_DUTY_MAX 100U
#define PCA9685_FULL_ON 0x10U
#define PCA9685_FULL_OFF 0x10U
#define PCA9685_SERVO_MIN 102U /* Environ 0,5 ms à 50 Hz; à calibrer. */
#define PCA9685_SERVO_MAX 512U /* Environ 2,5 ms à 50 Hz; à calibrer. */
#define PCA9685_SERVO_ANGLE_MAX 180U

static uint8_t PCA9685_EcrireRegistre(uint8_t registre, uint8_t valeur)
{
    uint8_t resultat = I2C_Start();
    uint8_t resultatStop;

    if (resultat != I2C_OK)
    {
        return resultat;
    }

    resultat = I2C_AdresseEcriture(PCA9685_ADRESSE);
    if (resultat == I2C_OK)
    {
        I2C_EffacerADDR();
        resultat = I2C_EnvoyerDonnee(registre);
    }
    if (resultat == I2C_OK)
    {
        resultat = I2C_EnvoyerDonnee(valeur);
    }

    if (resultat == I2C_ERREUR_ARLO)
    {
        return resultat;
    }

    resultatStop = I2C_Stop();
    if (resultat == I2C_OK)
    {
        resultat = resultatStop;
    }

    return resultat;
}

uint8_t PCA9685_TesterCommunication(void)
{
    uint8_t resultat = I2C_Start();
    uint8_t resultatStop;

    if (resultat != I2C_OK)
    {
        return resultat;
    }

    resultat = I2C_AdresseEcriture(PCA9685_ADRESSE);
    if (resultat == I2C_OK)
    {
        I2C_EffacerADDR();
    }
    if (resultat == I2C_ERREUR_ARLO)
    {
        return resultat;
    }

    resultatStop = I2C_Stop();
    if (resultat == I2C_OK)
    {
        resultat = resultatStop;
    }

    return resultat;
}

uint8_t PCA9685_ConfigurerPWM0(void)
{
    uint8_t resultat;

    resultat = PCA9685_EcrireRegistre(PCA9685_REG_MODE1,
                                     PCA9685_MODE1_SLEEP);
    if (resultat != I2C_OK)
    {
        return resultat;
    }

    resultat = PCA9685_EcrireRegistre(PCA9685_REG_PRESCALE,
                                     PCA9685_PRESCALE_50_HZ);
    if (resultat != I2C_OK)
    {
        return resultat;
    }

    resultat = PCA9685_EcrireRegistre(PCA9685_REG_MODE1,
                                     PCA9685_MODE1_AUTO_INCREMENT);
    if (resultat != I2C_OK)
    {
        return resultat;
    }

    SysTick_DelaiMs(1UL);

    resultat = PCA9685_EcrireRegistre(PCA9685_REG_LED0_ON_L,
                                     PCA9685_PWM0_ON_L);
    if (resultat != I2C_OK)
    {
        return resultat;
    }

    resultat = PCA9685_EcrireRegistre(PCA9685_REG_LED0_ON_H,
                                     PCA9685_PWM0_ON_H);
    if (resultat != I2C_OK)
    {
        return resultat;
    }

    resultat = PCA9685_EcrireRegistre(PCA9685_REG_LED0_OFF_L,
                                     PCA9685_PWM0_OFF_L);
    if (resultat != I2C_OK)
    {
        return resultat;
    }

    resultat = PCA9685_EcrireRegistre(PCA9685_REG_LED0_OFF_H,
                                     PCA9685_PWM0_OFF_H);
    if (resultat != I2C_OK)
    {
        return resultat;
    }

    return PCA9685_EcrireRegistre(PCA9685_REG_MODE1,
                                  PCA9685_MODE1_RESTART
                                | PCA9685_MODE1_AUTO_INCREMENT);
}

void PCA9685_SetDutyCycle(uint8_t canal, uint8_t dutyCycle)
{
    uint16_t valeur;
    uint8_t registre;
    uint8_t onH = 0U;
    uint8_t offL = 0U;
    uint8_t offH = 0U;

    if ((canal >= PCA9685_NOMBRE_CANAUX) || (dutyCycle > 100U))
    {
        return;
    }

    valeur = ((uint32_t)dutyCycle * PCA9685_PWM_MAX) / PCA9685_DUTY_MAX;

    if (dutyCycle == 0U)
    {
        offH = PCA9685_FULL_OFF;
    }
    else if (dutyCycle == 100U)
    {
        onH = PCA9685_FULL_ON;
    }
    else
    {
        offL = (uint8_t)(valeur & 0xFFU);
        offH = (uint8_t)((valeur >> 8U) & 0x0FU);
    }

    registre = PCA9685_REG_LED0_ON_L + (canal * 4U);

    PCA9685_EcrireRegistre(registre, 0U);
    PCA9685_EcrireRegistre(registre + 1U, onH);
    PCA9685_EcrireRegistre(registre + 2U, offL);
    PCA9685_EcrireRegistre(registre + 3U, offH);
}

void PCA9685_SetServoAngle(uint8_t canal, uint8_t angle)
{
    uint16_t valeur;
    uint8_t registre;

    if ((canal >= PCA9685_NOMBRE_CANAUX) ||
        (angle > PCA9685_SERVO_ANGLE_MAX))
    {
        return;
    }

    valeur = PCA9685_SERVO_MIN +
             ((uint32_t)angle *
             (PCA9685_SERVO_MAX - PCA9685_SERVO_MIN)) / 180UL;

    registre = PCA9685_REG_LED0_ON_L + (canal * 4U);

    PCA9685_EcrireRegistre(registre, 0U);
    PCA9685_EcrireRegistre(registre + 1U, 0U);
    PCA9685_EcrireRegistre(registre + 2U, (uint8_t)(valeur & 0xFFU));
    PCA9685_EcrireRegistre(registre + 3U, (uint8_t)(valeur >> 8U));
}
