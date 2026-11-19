
#include "I2C.h" // Inclut les fonctions et constantes de la communication I2C
#include <stdint.h> // Permet d'utiliser les types uint8_t et uint32_t

// Registres de gestion des horloges et des périphériques du STM32F103
#define RCC_APB2ENR (*(volatile uint32_t *)0x40021018UL) // Active les horloges GPIO et AFIO
#define RCC_APB1ENR (*(volatile uint32_t *)0x4002101CUL) // Active l'horloge du périphérique I2C1
#define RCC_APB1RSTR (*(volatile uint32_t *)0x40021010UL) // Réinitialise les périphériques APB1
#define AFIO_MAPR   (*(volatile uint32_t *)0x40010004UL) // Configure le remappage des broches

// Registres du port GPIOB
#define GPIOB_CRL   (*(volatile uint32_t *)0x40010C00UL) // Configure les broches PB0 à PB7
#define GPIOB_IDR   (*(volatile uint32_t *)0x40010C08UL) // Lit l'état des broches
#define GPIOB_BSRR  (*(volatile uint32_t *)0x40010C10UL) // Met les sorties à 1 ou à 0

// Registres du périphérique I2C1
#define I2C1_CR1    (*(volatile uint32_t *)0x40005400UL) // Registre de contrôle principal
#define I2C1_CR2    (*(volatile uint32_t *)0x40005404UL) // Registre de contrôle secondaire
#define I2C1_DR     (*(volatile uint32_t *)0x40005410UL) // Registre des données transmises ou reçues
#define I2C1_SR1    (*(volatile uint32_t *)0x40005414UL) // Registre des indicateurs et erreurs
#define I2C1_SR2    (*(volatile uint32_t *)0x40005418UL) // Registre de l'état du bus I2C
#define I2C1_CCR    (*(volatile uint32_t *)0x4000541CUL) // Configure la vitesse de communication
#define I2C1_TRISE  (*(volatile uint32_t *)0x40005420UL) // Configure le temps de montée maximal

// Masques pour activer les horloges et réinitialiser I2C1
#define AFIO_CLOCK_ENABLE   (1UL << 0U) // Active l'horloge AFIO
#define GPIOB_CLOCK_ENABLE  (1UL << 3U) // Active l'horloge GPIOB
#define I2C1_CLOCK_ENABLE   (1UL << 21U) // Active l'horloge I2C1
#define I2C1_RESET          (1UL << 21U) // Réinitialise le périphérique I2C1

// Masque de remappage des broches I2C1
#define I2C1_REMAP          (1UL << 1U) // Sélectionne PB8/PB9 au lieu de PB6/PB7 si activé

// Bits de configuration du registre I2C1_CR1
#define I2C_CR1_PE          (1UL << 0U) // Active le périphérique I2C
#define I2C_CR1_SWRST       (1UL << 15U) // Effectue une réinitialisation logicielle
#define I2C_CR1_START       (1UL << 8U) // Génère une condition START
#define I2C_CR1_STOP        (1UL << 9U) // Génère une condition STOP

// Masques pour configurer la fréquence et les temporisations
#define I2C_CR2_FREQ_MASK   0x3FUL // Masque des bits de fréquence APB1
#define I2C_CCR_MASK        0x0FFFUL // Masque de la valeur du diviseur d'horloge
#define I2C_TRISE_MASK      0x3FUL // Masque du temps de montée maximal

// Indicateurs du registre I2C1_SR1
#define I2C_SR1_SB           (1UL << 0U) // Condition START générée
#define I2C_SR1_ADDR         (1UL << 1U) // Adresse envoyée et acquittée
#define I2C_SR1_BTF          (1UL << 2U) // Transfert d'octet terminé
#define I2C_SR1_TXE          (1UL << 7U) // Registre de transmission vide
#define I2C_SR1_BERR         (1UL << 8U) // Erreur de communication sur le bus
#define I2C_SR1_ARLO         (1UL << 9U) // Perte d'arbitrage
#define I2C_SR1_AF           (1UL << 10U) // Aucun acquittement reçu (NACK)

// Indicateurs du registre I2C1_SR2
#define I2C_SR2_MSL          (1UL << 0U) // Indique que le STM32 est maître
#define I2C_SR2_BUSY         (1UL << 1U) // Indique que le bus I2C est occupé

// Masques de configuration des broches GPIO
#define GPIO_PB6_CONFIG     (0xFUL << 24U) // Masque des 4 bits de configuration de PB6
#define GPIO_PB7_CONFIG     (0xFUL << 28U) // Masque des 4 bits de configuration de PB7
#define GPIO_PB6_PB7_MASK   (GPIO_PB6_CONFIG | GPIO_PB7_CONFIG) // Masque des deux broches
#define GPIO_PB6_PB7_OD     ((0x6UL << 24U) | (0x6UL << 28U)) // Configure PB6 et PB7 en sorties open-drain à 2 MHz

// Broches utilisées pour la communication I2C
#define I2C_SCL_PIN         (1UL << 6U) // PB6 : signal d'horloge SCL
#define I2C_SDA_PIN         (1UL << 7U) // PB7 : signal de données SDA
#define I2C_LIGNES_MASK     (I2C_SCL_PIN | I2C_SDA_PIN) // Masque des deux lignes I2C

// Paramètres de fréquence et de temporisation
#define I2C_PCLK1_MHZ       8UL // Fréquence APB1 supposée de 8 MHz
#define I2C_STANDARD_HZ     100000UL // Vitesse I2C standard de 100 kHz
#define I2C_MAX_ADRESSE     0x7FU // Adresse I2C maximale sur 7 bits
#define I2C_TIMEOUT_COMPTE  100000UL // Nombre maximal d'itérations avant une erreur

// Attend qu'un indicateur du registre SR1 soit activé
static uint8_t I2C_AttendreSR1(uint32_t drapeau)
{
    uint32_t compteur = I2C_TIMEOUT_COMPTE; // Initialise le compteur de temporisation

    while ((I2C1_SR1 & drapeau) == 0UL) // Attend l'activation du drapeau demandé
    {
        uint32_t etat = I2C1_SR1; // Lit les indicateurs du périphérique I2C1

        if ((etat & I2C_SR1_BERR) != 0UL) // Vérifie une erreur sur le bus
        {
            return I2C_ERREUR_BERR; // Retourne une erreur de bus
        }
        if ((etat & I2C_SR1_ARLO) != 0UL) // Vérifie une perte d'arbitrage
        {
            return I2C_ERREUR_ARLO; // Retourne une erreur d'arbitrage
        }
        if ((etat & I2C_SR1_AF) != 0UL) // Vérifie l'absence d'acquittement
        {
            return I2C_ERREUR_NACK; // Retourne une erreur NACK
        }
        if (compteur == 0UL) // Vérifie si le délai maximal est atteint
        {
            return I2C_ERREUR_TIMEOUT; // Retourne une erreur de temporisation
        }
        compteur--; // Décrémente le compteur
    }

    return I2C_OK; // Indique que le drapeau attendu est activé
}

// Attend que le bus I2C soit disponible
static uint8_t I2C_AttendreBusLibre(void)
{
    uint32_t compteur = I2C_TIMEOUT_COMPTE; // Initialise le compteur de temporisation

    while ((I2C1_SR2 & I2C_SR2_BUSY) != 0UL) // Vérifie si le bus est occupé
    {
        if (compteur == 0UL) // Vérifie si le délai maximal est atteint
        {
            return I2C_ERREUR_BUS_OCCUPE; // Retourne une erreur de bus occupé
        }
        compteur--; // Décrémente le compteur
    }

    return I2C_OK; // Indique que le bus est libre
}

// Génère un court délai logiciel
static void I2C_DelaiCourt(void)
{
    volatile uint32_t compteur; // Compteur conservé pour la temporisation

    for (compteur = 0UL; compteur < 100UL; compteur++) // Effectue 100 itérations
    {
        __asm volatile ("nop"); // Exécute une instruction sans opération
    }
}

// Attend que la ligne SCL retourne au niveau haut
static uint8_t I2C_AttendreSCL(void)
{
    uint32_t compteur = I2C_TIMEOUT_COMPTE; // Initialise le compteur de temporisation

    while ((GPIOB_IDR & I2C_SCL_PIN) == 0UL) // Attend que SCL soit au niveau haut
    {
        if (compteur == 0UL) // Vérifie si le délai maximal est atteint
        {
            return 0U; // Indique que SCL est restée au niveau bas
        }
        compteur--; // Décrémente le compteur
    }

    return 1U; // Indique que SCL est au niveau haut
}

// Tente de récupérer un bus I2C bloqué
static uint8_t I2C_RecupererBus(void)
{
    uint32_t impulsion; // Compteur des impulsions d'horloge
    uint8_t resultat = I2C_OK; // Initialise le résultat de la récupération

    // Désactive I2C1 pour contrôler directement les broches
    I2C1_CR1 &= ~I2C_CR1_PE;

    // Configure PB6 et PB7 en sorties open-drain
    GPIOB_CRL = (GPIOB_CRL & ~GPIO_PB6_PB7_MASK) | GPIO_PB6_PB7_OD;

    GPIOB_BSRR = I2C_LIGNES_MASK; // Relâche les lignes SCL et SDA
    I2C_DelaiCourt(); // Attend la stabilisation des lignes

    // Génère jusqu'à 9 impulsions d'horloge si SDA reste au niveau bas
    for (impulsion = 0UL;
         (impulsion < 9UL) && ((GPIOB_IDR & I2C_SDA_PIN) == 0UL);
         impulsion++)
    {
        GPIOB_BSRR = I2C_SCL_PIN << 16U; // Force SCL au niveau bas
        I2C_DelaiCourt(); // Maintient brièvement SCL au niveau bas

        GPIOB_BSRR = I2C_SCL_PIN; // Relâche SCL au niveau haut

        if (I2C_AttendreSCL() == 0U) // Vérifie si SCL peut remonter
        {
            resultat = I2C_ERREUR_BUS_OCCUPE; // Signale un blocage de SCL
            break; // Arrête la génération des impulsions
        }
        I2C_DelaiCourt(); // Maintient brièvement SCL au niveau haut
    }

    // Tente de générer une condition STOP pour libérer le bus
    GPIOB_BSRR = I2C_LIGNES_MASK << 16U; // Force SCL et SDA au niveau bas
    I2C_DelaiCourt(); // Attend brièvement

    GPIOB_BSRR = I2C_SCL_PIN; // Relâche SCL au niveau haut

    if (I2C_AttendreSCL() == 0U) // Vérifie si SCL est libérée
    {
        resultat = I2C_ERREUR_BUS_OCCUPE; // Signale un blocage du bus
    }

    I2C_DelaiCourt(); // Attend avant de relâcher SDA
    GPIOB_BSRR = I2C_SDA_PIN; // Relâche SDA pour terminer la condition STOP
    I2C_DelaiCourt(); // Attend la stabilisation des lignes

    // Rétablit la configuration des broches pour le périphérique I2C
    GPIOB_CRL = (GPIOB_CRL & ~GPIO_PB6_PB7_MASK)
              | GPIO_PB6_CONFIG | GPIO_PB7_CONFIG;

    I2C_Init(); // Réinitialise et reconfigure le périphérique I2C1

    // Vérifie que les deux lignes sont au niveau haut
    if ((GPIOB_IDR & I2C_LIGNES_MASK) != I2C_LIGNES_MASK)
    {
        return I2C_ERREUR_BUS_OCCUPE; // Indique qu'une ligne reste bloquée
    }

    if (resultat != I2C_OK) // Vérifie si une erreur est survenue
    {
        return resultat; // Retourne l'erreur détectée
    }

    return I2C_AttendreBusLibre(); // Vérifie que le bus est disponible
}

// Efface les indicateurs d'erreur du périphérique I2C1
static void I2C_EffacerErreurs(void)
{
    I2C1_SR1 &= ~(I2C_SR1_BERR | I2C_SR1_ARLO | I2C_SR1_AF); // Efface les erreurs BERR, ARLO et AF
}

// Initialise le périphérique I2C1 en mode standard à 100 kHz
void I2C_Init(void)
{
    // Active les horloges nécessaires à la communication I2C
    RCC_APB2ENR |= AFIO_CLOCK_ENABLE; // Active l'horloge AFIO
    RCC_APB2ENR |= GPIOB_CLOCK_ENABLE; // Active l'horloge GPIOB

    // Sélectionne les broches I2C1 par défaut : PB6 = SCL et PB7 = SDA
    AFIO_MAPR &= ~I2C1_REMAP; // Désactive le remappage I2C1

    // Active l'horloge et réinitialise le périphérique I2C1
    RCC_APB1ENR |= I2C1_CLOCK_ENABLE; // Active l'horloge I2C1
    RCC_APB1RSTR |= I2C1_RESET; // Active la réinitialisation matérielle
    RCC_APB1RSTR &= ~I2C1_RESET; // Termine la réinitialisation

    // Désactive I2C1 avant de modifier sa configuration
    I2C1_CR1 &= ~I2C_CR1_PE; // Désactive le périphérique

    // Configure PB6 et PB7 en fonction alternative open-drain à 50 MHz
    GPIOB_CRL &= ~(GPIO_PB6_CONFIG | GPIO_PB7_CONFIG); // Efface l'ancienne configuration
    GPIOB_CRL |=  (GPIO_PB6_CONFIG | GPIO_PB7_CONFIG); // Applique la configuration I2C

    // Effectue une réinitialisation logicielle du périphérique
    I2C1_CR1 |= I2C_CR1_SWRST; // Active la réinitialisation logicielle
    I2C1_CR1 &= ~I2C_CR1_SWRST; // Termine la réinitialisation logicielle

    // Configure la fréquence d'horloge APB1
    I2C1_CR2 &= ~I2C_CR2_FREQ_MASK; // Efface les anciens bits de fréquence
    I2C1_CR2 |= I2C_PCLK1_MHZ; // Configure la fréquence APB1 à 8 MHz

    // Calcule le diviseur d'horloge pour une communication à 100 kHz
    uint32_t ccr = (I2C_PCLK1_MHZ * 1000000UL)
                 / (2UL * I2C_STANDARD_HZ);

    // Configure le registre CCR en mode standard
    I2C1_CCR = ccr & I2C_CCR_MASK; // Applique le diviseur de fréquence calculé

    // Calcule le temps de montée maximal en mode standard
    uint32_t trise = I2C_PCLK1_MHZ + 1UL;

    I2C1_TRISE = trise & I2C_TRISE_MASK; // Configure le registre TRISE

    // Active le périphérique I2C1
    I2C1_CR1 |= I2C_CR1_PE; // Permet le fonctionnement du bus I2C
}

// Génère une condition START pour commencer une communication I2C
uint8_t I2C_Start(void)
{
    uint8_t resultat = I2C_AttendreBusLibre(); // Attend que le bus soit disponible

    if (resultat != I2C_OK) // Vérifie si le bus est bloqué
    {
        resultat = I2C_RecupererBus(); // Tente de récupérer le bus
        if (resultat != I2C_OK) // Vérifie si la récupération a échoué
        {
            return resultat; // Retourne l'erreur rencontrée
        }
    }

    I2C_EffacerErreurs(); // Efface les anciennes erreurs
    I2C1_CR1 |= I2C_CR1_START; // Demande une condition START

    resultat = I2C_AttendreSR1(I2C_SR1_SB); // Attend la génération du START
    if (resultat != I2C_OK) // Vérifie si une erreur est survenue
    {
        if (resultat != I2C_ERREUR_ARLO) // Vérifie si l'arbitrage n'a pas été perdu
        {
            I2C_Stop(); // Tente de terminer la communication
        }
        I2C_EffacerErreurs(); // Efface les indicateurs d'erreur
    }

    return resultat; // Retourne le résultat de la condition START
}

// Transmet l'adresse I2C d'un périphérique en mode écriture
uint8_t I2C_AdresseEcriture(uint8_t adresse)
{
    if (adresse > I2C_MAX_ADRESSE) // Vérifie que l'adresse tient sur 7 bits
    {
        return I2C_ERREUR_ADRESSE; // Retourne une erreur d'adresse
    }
    if ((I2C1_SR1 & I2C_SR1_SB) == 0UL) // Vérifie qu'une condition START a été générée
    {
        return I2C_ERREUR_SEQUENCE; // Retourne une erreur de séquence
    }

    I2C1_DR = (uint32_t)adresse << 1U; // Transmet l'adresse avec le bit écriture à 0
    return I2C_AttendreSR1(I2C_SR1_ADDR); // Attend l'acquittement de l'adresse
}

// Efface le drapeau ADDR après la transmission de l'adresse
void I2C_EffacerADDR(void)
{
    volatile uint32_t lecture; // Variable utilisée pour lire les registres

    lecture = I2C1_SR1; // Première lecture nécessaire pour effacer ADDR
    lecture = I2C1_SR2; // Deuxième lecture qui termine l'effacement
    (void)lecture; // Évite un avertissement de variable inutilisée
}

// Transmet un octet de données sur le bus I2C
uint8_t I2C_EnvoyerDonnee(uint8_t donnee)
{
    uint8_t resultat; // Stocke le résultat des vérifications

    if ((I2C1_SR2 & I2C_SR2_MSL) == 0UL) // Vérifie que le STM32 est maître
    {
        return I2C_ERREUR_SEQUENCE; // Retourne une erreur de séquence
    }

    resultat = I2C_AttendreSR1(I2C_SR1_TXE); // Attend que le registre de transmission soit vide
    if (resultat != I2C_OK) // Vérifie si une erreur est survenue
    {
        return resultat; // Retourne l'erreur détectée
    }

    I2C1_DR = donnee; // Charge l'octet dans le registre de transmission
    return I2C_AttendreSR1(I2C_SR1_BTF); // Attend la fin du transfert
}

// Génère une condition STOP pour terminer la communication I2C
uint8_t I2C_Stop(void)
{
    uint8_t resultat; // Stocke le résultat de l'opération

    I2C1_CR1 |= I2C_CR1_STOP; // Demande une condition STOP
    I2C1_SR1 &= ~I2C_SR1_AF; // Efface l'indicateur d'absence d'acquittement

    resultat = I2C_AttendreBusLibre(); // Attend que le bus soit libéré
    if (resultat != I2C_OK) // Vérifie si le bus reste occupé
    {
        resultat = I2C_RecupererBus(); // Tente de récupérer le bus
    }

    return resultat; // Retourne le résultat de l'opération
}

// Effectue une transmission complète d'un octet vers un périphérique I2C
uint8_t I2C_EcrireOctet(uint8_t adresse, uint8_t donnee)
{
    uint8_t resultat = I2C_Start(); // Génère une condition START
    uint8_t resultatStop; // Stocke le résultat de la condition STOP

    if (resultat != I2C_OK) // Vérifie si START a échoué
    {
        return resultat; // Retourne l'erreur rencontrée
    }

    resultat = I2C_AdresseEcriture(adresse); // Transmet l'adresse du périphérique
    if (resultat == I2C_OK) // Vérifie si le périphérique a répondu
    {
        I2C_EffacerADDR(); // Efface le drapeau ADDR
        resultat = I2C_EnvoyerDonnee(donnee); // Transmet l'octet de données
    }

    if (resultat == I2C_ERREUR_ARLO) // Vérifie une perte d'arbitrage
    {
        I2C_EffacerErreurs(); // Efface les indicateurs d'erreur
        return resultat; // Retourne l'erreur d'arbitrage
    }

    resultatStop = I2C_Stop(); // Génère une condition STOP
    if (resultat == I2C_OK) // Vérifie si la transmission a réussi
    {
        resultat = resultatStop; // Conserve le résultat de la condition STOP
    }

    return resultat; // Retourne le résultat final de la transmission
}
