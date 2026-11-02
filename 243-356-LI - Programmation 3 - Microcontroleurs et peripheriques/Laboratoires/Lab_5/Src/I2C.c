
#include "I2C.h"
#include <stdint.h>

// Registres et masques pour I2C1 sur STM32F103
#define RCC_APB2ENR (*(volatile uint32_t *)0x40021018UL)
#define RCC_APB1ENR (*(volatile uint32_t *)0x4002101CUL)
#define RCC_APB1RSTR (*(volatile uint32_t *)0x40021010UL)
#define AFIO_MAPR   (*(volatile uint32_t *)0x40010004UL)
#define GPIOB_CRL   (*(volatile uint32_t *)0x40010C00UL)
#define GPIOB_IDR   (*(volatile uint32_t *)0x40010C08UL)
#define GPIOB_BSRR  (*(volatile uint32_t *)0x40010C10UL)
#define I2C1_CR1    (*(volatile uint32_t *)0x40005400UL)
#define I2C1_CR2    (*(volatile uint32_t *)0x40005404UL)
#define I2C1_DR     (*(volatile uint32_t *)0x40005410UL)
#define I2C1_SR1    (*(volatile uint32_t *)0x40005414UL)
#define I2C1_SR2    (*(volatile uint32_t *)0x40005418UL)
#define I2C1_CCR    (*(volatile uint32_t *)0x4000541CUL)
#define I2C1_TRISE  (*(volatile uint32_t *)0x40005420UL)

// Activation des horloges
#define AFIO_CLOCK_ENABLE   (1UL << 0U)
#define GPIOB_CLOCK_ENABLE  (1UL << 3U)
#define I2C1_CLOCK_ENABLE   (1UL << 21U)
#define I2C1_RESET          (1UL << 21U)

// Remappage I2C1
#define I2C1_REMAP          (1UL << 1U)

// Configuration I2C1
#define I2C_CR1_PE          (1UL << 0U)
#define I2C_CR1_SWRST       (1UL << 15U)
#define I2C_CR1_START       (1UL << 8U)
#define I2C_CR1_STOP        (1UL << 9U)
#define I2C_CR2_FREQ_MASK   0x3FUL
#define I2C_CCR_MASK        0x0FFFUL
#define I2C_TRISE_MASK      0x3FUL

#define I2C_SR1_SB           (1UL << 0U)
#define I2C_SR1_ADDR         (1UL << 1U)
#define I2C_SR1_BTF          (1UL << 2U)
#define I2C_SR1_TXE          (1UL << 7U)
#define I2C_SR1_BERR         (1UL << 8U)
#define I2C_SR1_ARLO         (1UL << 9U)
#define I2C_SR1_AF           (1UL << 10U)
#define I2C_SR2_MSL          (1UL << 0U)
#define I2C_SR2_BUSY         (1UL << 1U)

// Configuration des GPIO
#define GPIO_PB6_CONFIG     (0xFUL << 24U)
#define GPIO_PB7_CONFIG     (0xFUL << 28U)
#define GPIO_PB6_PB7_MASK   (GPIO_PB6_CONFIG | GPIO_PB7_CONFIG)
#define GPIO_PB6_PB7_OD     ((0x6UL << 24U) | (0x6UL << 28U))
#define I2C_SCL_PIN         (1UL << 6U)
#define I2C_SDA_PIN         (1UL << 7U)
#define I2C_LIGNES_MASK     (I2C_SCL_PIN | I2C_SDA_PIN)

// Fréquences
#define I2C_PCLK1_MHZ       8UL
#define I2C_STANDARD_HZ     100000UL
#define I2C_MAX_ADRESSE     0x7FU
#define I2C_TIMEOUT_COMPTE  100000UL

static uint8_t I2C_AttendreSR1(uint32_t drapeau)
{
    uint32_t compteur = I2C_TIMEOUT_COMPTE;

    while ((I2C1_SR1 & drapeau) == 0UL)
    {
        uint32_t etat = I2C1_SR1;

        if ((etat & I2C_SR1_BERR) != 0UL)
        {
            return I2C_ERREUR_BERR;
        }
        if ((etat & I2C_SR1_ARLO) != 0UL)
        {
            return I2C_ERREUR_ARLO;
        }
        if ((etat & I2C_SR1_AF) != 0UL)
        {
            return I2C_ERREUR_NACK;
        }
        if (compteur == 0UL)
        {
            return I2C_ERREUR_TIMEOUT;
        }
        compteur--;
    }

    return I2C_OK;
}

static uint8_t I2C_AttendreBusLibre(void)
{
    uint32_t compteur = I2C_TIMEOUT_COMPTE;

    while ((I2C1_SR2 & I2C_SR2_BUSY) != 0UL)
    {
        if (compteur == 0UL)
        {
            return I2C_ERREUR_BUS_OCCUPE;
        }
        compteur--;
    }

    return I2C_OK;
}

static void I2C_DelaiCourt(void)
{
    volatile uint32_t compteur;

    for (compteur = 0UL; compteur < 100UL; compteur++)
    {
        __asm volatile ("nop");
    }
}

static uint8_t I2C_AttendreSCL(void)
{
    uint32_t compteur = I2C_TIMEOUT_COMPTE;

    while ((GPIOB_IDR & I2C_SCL_PIN) == 0UL)
    {
        if (compteur == 0UL)
        {
            return 0U;
        }
        compteur--;
    }

    return 1U;
}

static uint8_t I2C_RecupererBus(void)
{
    uint32_t impulsion;
    uint8_t resultat = I2C_OK;

    /* I2C1 est coupé pendant que PB6/PB7 produisent jusqu'à neuf clocks. */
    I2C1_CR1 &= ~I2C_CR1_PE;
    GPIOB_CRL = (GPIOB_CRL & ~GPIO_PB6_PB7_MASK) | GPIO_PB6_PB7_OD;
    GPIOB_BSRR = I2C_LIGNES_MASK; /* relâche SCL et SDA */
    I2C_DelaiCourt();

    for (impulsion = 0UL;
         (impulsion < 9UL) && ((GPIOB_IDR & I2C_SDA_PIN) == 0UL);
         impulsion++)
    {
        GPIOB_BSRR = I2C_SCL_PIN << 16U; /* SCL à zéro */
        I2C_DelaiCourt();
        GPIOB_BSRR = I2C_SCL_PIN; /* relâche SCL */
        if (I2C_AttendreSCL() == 0U)
        {
            resultat = I2C_ERREUR_BUS_OCCUPE;
            break;
        }
        I2C_DelaiCourt();
    }

    /* Génère STOP: SDA remonte pendant que SCL est relâchée. */
    GPIOB_BSRR = I2C_LIGNES_MASK << 16U;
    I2C_DelaiCourt();
    GPIOB_BSRR = I2C_SCL_PIN;
    if (I2C_AttendreSCL() == 0U)
    {
        resultat = I2C_ERREUR_BUS_OCCUPE;
    }
    I2C_DelaiCourt();
    GPIOB_BSRR = I2C_SDA_PIN;
    I2C_DelaiCourt();

    GPIOB_CRL = (GPIOB_CRL & ~GPIO_PB6_PB7_MASK)
              | GPIO_PB6_CONFIG | GPIO_PB7_CONFIG;
    I2C_Init(); /* réinitialise aussi les registres après une erreur */

    if ((GPIOB_IDR & I2C_LIGNES_MASK) != I2C_LIGNES_MASK)
    {
        return I2C_ERREUR_BUS_OCCUPE;
    }

    if (resultat != I2C_OK)
    {
        return resultat;
    }

    return I2C_AttendreBusLibre();
}

static void I2C_EffacerErreurs(void)
{
    I2C1_SR1 &= ~(I2C_SR1_BERR | I2C_SR1_ARLO | I2C_SR1_AF);
}

void I2C_Init(void)
{
    // 1. Activer les horloges AFIO et GPIOB
    RCC_APB2ENR |= AFIO_CLOCK_ENABLE;
    RCC_APB2ENR |= GPIOB_CLOCK_ENABLE;

    // 2. Utiliser les broches par défaut
    // PB6 = SCL, PB7 = SDA
    AFIO_MAPR &= ~I2C1_REMAP;

    // 3. Activer puis réinitialiser le périphérique I2C1
    RCC_APB1ENR |= I2C1_CLOCK_ENABLE;
    RCC_APB1RSTR |= I2C1_RESET;
    RCC_APB1RSTR &= ~I2C1_RESET;

    // 4. Désactiver I2C1 avant configuration
    I2C1_CR1 &= ~I2C_CR1_PE;

    // 5. Configurer PB6 et PB7
    // Alternate Function Open-Drain, 50 MHz
    GPIOB_CRL &= ~(GPIO_PB6_CONFIG | GPIO_PB7_CONFIG);
    GPIOB_CRL |=  (GPIO_PB6_CONFIG | GPIO_PB7_CONFIG);

    /* Le reset logiciel libère BUSY une fois les broches stables. */
    I2C1_CR1 |= I2C_CR1_SWRST;
    I2C1_CR1 &= ~I2C_CR1_SWRST;

    // 6. Configurer CR2 avec la fréquence PCLK1
    I2C1_CR2 &= ~I2C_CR2_FREQ_MASK;
    I2C1_CR2 |= I2C_PCLK1_MHZ;

    // 7. Calculer CCR pour 100 kHz
    uint32_t ccr = (I2C_PCLK1_MHZ * 1000000UL)
                 / (2UL * I2C_STANDARD_HZ);

    // Mode Standard (F/S = 0)
    I2C1_CCR = ccr & I2C_CCR_MASK;

    // 8. Configurer TRISE
    // En mode Standard : PCLK1(MHz) + 1
    uint32_t trise = I2C_PCLK1_MHZ + 1UL;

    I2C1_TRISE = trise & I2C_TRISE_MASK;

    // 9. Activer I2C1
    I2C1_CR1 |= I2C_CR1_PE;
}

uint8_t I2C_Start(void)
{
    uint8_t resultat = I2C_AttendreBusLibre();

    if (resultat != I2C_OK)
    {
        resultat = I2C_RecupererBus();
        if (resultat != I2C_OK)
        {
            return resultat;
        }
    }

    I2C_EffacerErreurs();
    I2C1_CR1 |= I2C_CR1_START;

    resultat = I2C_AttendreSR1(I2C_SR1_SB);
    if (resultat != I2C_OK)
    {
        if (resultat != I2C_ERREUR_ARLO)
        {
            I2C_Stop();
        }
        I2C_EffacerErreurs();
    }

    return resultat;
}

uint8_t I2C_AdresseEcriture(uint8_t adresse)
{
    if (adresse > I2C_MAX_ADRESSE)
    {
        return I2C_ERREUR_ADRESSE;
    }
    if ((I2C1_SR1 & I2C_SR1_SB) == 0UL)
    {
        return I2C_ERREUR_SEQUENCE;
    }

    I2C1_DR = (uint32_t)adresse << 1U;
    return I2C_AttendreSR1(I2C_SR1_ADDR);
}

void I2C_EffacerADDR(void)
{
    volatile uint32_t lecture;

    lecture = I2C1_SR1;
    lecture = I2C1_SR2;
    (void)lecture;
}

uint8_t I2C_EnvoyerDonnee(uint8_t donnee)
{
    uint8_t resultat;

    if ((I2C1_SR2 & I2C_SR2_MSL) == 0UL)
    {
        return I2C_ERREUR_SEQUENCE;
    }

    resultat = I2C_AttendreSR1(I2C_SR1_TXE);
    if (resultat != I2C_OK)
    {
        return resultat;
    }

    I2C1_DR = donnee;
    return I2C_AttendreSR1(I2C_SR1_BTF);
}

uint8_t I2C_Stop(void)
{
    uint8_t resultat;

    I2C1_CR1 |= I2C_CR1_STOP;
    I2C1_SR1 &= ~I2C_SR1_AF;

    resultat = I2C_AttendreBusLibre();
    if (resultat != I2C_OK)
    {
        resultat = I2C_RecupererBus();
    }

    return resultat;
}

uint8_t I2C_EcrireOctet(uint8_t adresse, uint8_t donnee)
{
    uint8_t resultat = I2C_Start();
    uint8_t resultatStop;

    if (resultat != I2C_OK)
    {
        return resultat;
    }

    resultat = I2C_AdresseEcriture(adresse);
    if (resultat == I2C_OK)
    {
        I2C_EffacerADDR();
        resultat = I2C_EnvoyerDonnee(donnee);
    }

    if (resultat == I2C_ERREUR_ARLO)
    {
        I2C_EffacerErreurs();
        return resultat;
    }

    resultatStop = I2C_Stop();
    if (resultat == I2C_OK)
    {
        resultat = resultatStop;
    }

    return resultat;
}
