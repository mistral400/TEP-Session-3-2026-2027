
#include "gpio.h" // Inclut les déclarations des fonctions et constantes GPIO


#define RCC_APB2ENR (*(volatile uint32_t *)0x40021018UL)
#define GPIOA_CRL (*(volatile uint32_t *)0x40010800UL)
#define GPIOB_CRL  (*(volatile uint32_t *)0x40010C00UL) // Configuration des broches PB0 à PB7
#define GPIOB_IDR  (*(volatile uint32_t *)0x40010C08UL) // Lecture de l'état des entrées
#define GPIOB_BSRR (*(volatile uint32_t *)0x40010C10UL) // Mise à 1 ou à 0 des sorties
// PIN
#define LED_PIN     6u // LED connectée sur PB6
#define BUTTON_PIN  5u // Bouton connecté sur PB5


// MASQUES ET CONSTANTES POUR LA CONFIGURATION DES BROCHES
#define LED_MASK    (1u << LED_PIN) // Masque de la LED
#define BUTTON_MASK (1u << BUTTON_PIN) // Masque du bouton
#define GPIO_CONFIG_MASK   0xFu // Masque pour effacer les 4 bits de configuration
#define GPIO_PA0_CONFIG_MASK 0x0FUL
#define GPIOA_CLOCK_ENABLE (1 )

#define GPIOB_CLOCK_ENABLE (1u << 3u) // Active l'horloge du port GPIOB
#define GPIO_RESET_SHIFT   16u // Décalage pour remettre une sortie à 0

#define GPIO_CONFIG_BITS   4u // Chaque broche utilise 4 bits de configuration


#define LED_CONFIG_SHIFT    (LED_PIN * GPIO_CONFIG_BITS) // Position des bits de configuration de PB6
#define BUTTON_CONFIG_SHIFT (BUTTON_PIN * GPIO_CONFIG_BITS) // Position des bits de configuration de PB5

#define LED_OUTPUT_MODE  0x2u // Configure la LED en sortie push-pull à 2 MHz
#define BUTTON_INPUT_MODE 0x4u // Configure le bouton en entrée flottante

static uint8_t etatLED = LED_OFF; // Mémorise l'état actuel de la LED

void GPIOA_ConfigurerPA0Analogique(void)
{
    RCC_APB2ENR |= GPIOA_CLOCK_ENABLE;
    GPIOA_CRL &= ~GPIO_PA0_CONFIG_MASK;
}

void LED_Init(void) // Initialise la broche de la LED
{
    RCC_APB2ENR |= GPIOB_CLOCK_ENABLE; // Active l'horloge du port GPIOB

    GPIOB_CRL &= ~(GPIO_CONFIG_MASK << LED_CONFIG_SHIFT); // Efface l'ancienne configuration de PB6
    GPIOB_CRL |= (LED_OUTPUT_MODE << LED_CONFIG_SHIFT); // Configure PB6 en sortie push-pull

    LED_Set(LED_OFF); // Éteint la LED au démarrage
}

void LED_Set(uint8_t etat) // Allume ou éteint la LED selon l'état demandé
{
    if (etat == LED_ON) // Vérifie si la LED doit être allumée
    {
        GPIOB_BSRR = (LED_MASK << GPIO_RESET_SHIFT); // Met PB6 à 0 pour allumer la LED
    }
    else // Si la LED doit être éteinte
    {
        GPIOB_BSRR = LED_MASK; // Met PB6 à 1 pour éteindre la LED
    }

    etatLED = etat; // Mémorise le nouvel état de la LED
}

void LED_Toggle(void) // Inverse l'état actuel de la LED
{
    if (etatLED == LED_OFF) // Vérifie si la LED est éteinte
    {
        LED_Set(LED_ON); // Allume la LED
    }
    else // Si la LED est déjà allumée
    {
        LED_Set(LED_OFF); // Éteint la LED
    }
}

void Button_Init(void) // Initialise la broche du bouton
{
    RCC_APB2ENR |= GPIOB_CLOCK_ENABLE; // Active l'horloge du port GPIOB

    GPIOB_CRL &= ~(GPIO_CONFIG_MASK << BUTTON_CONFIG_SHIFT); // Efface l'ancienne configuration de PB5
    GPIOB_CRL |= (BUTTON_INPUT_MODE << BUTTON_CONFIG_SHIFT); // Configure PB5 en entrée flottante
}

uint8_t Button_Read(void) // Lit l'état du bouton
{
    if ((GPIOB_IDR & BUTTON_MASK) == 0u) // Vérifie si PB5 est à 0 (bouton appuyé)
    {
        return 1u; // Retourne 1 si le bouton est appuyé
    }
    else // Si PB5 est à 1
    {
        return 0u; // Retourne 0 si le bouton n'est pas appuyé
    }
}
