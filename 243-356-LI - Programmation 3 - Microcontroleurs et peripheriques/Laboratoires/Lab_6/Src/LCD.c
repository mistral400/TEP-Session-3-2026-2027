
#include "LCD.h"

#define RCC_APB2ENR (*(volatile uint32_t *)0x40021018UL)

#define GPIOB_CRH  (*(volatile uint32_t *)0x40010C04UL)
#define GPIOB_BSRR (*(volatile uint32_t *)0x40010C10UL)

#define SYST_CVR (*(volatile uint32_t *)0xE000E018UL)

#define LCD_RS_PIN 9U
#define LCD_RW_PIN 10U
#define LCD_E_PIN  11U
#define LCD_D4_PIN 12U

#define LCD_DATA_MASK 0x0FUL
#define LCD_7_PINS_MASK (0x7FUL << LCD_RS_PIN)

#define GPIOB_CLOCK_ENABLE (1UL << 3)
#define GPIO_RESET_SHIFT   16U
#define GPIOB_CONFIG_MASK  0xFFFFFFF0UL
#define GPIOB_CONFIG_OUTPUT 0x22222220UL

#define SYSTICK_RELOAD     7999UL
#define SYSTICK_PERIODE    (SYSTICK_RELOAD + 1UL)
#define SYSTICK_DELAI_MAX_CYCLES (SYSTICK_PERIODE / 2UL)
#define CPU_HZ             8000000UL

#define US_PER_SECOND 1000000UL
#define US_PER_MS     1000UL

#define LCD_CMD_CLEAR        0x01U
#define LCD_CMD_HOME         0x02U
#define LCD_CMD_ENTRY_MODE   0x06U
#define LCD_CMD_DISPLAY_OFF  0x08U
#define LCD_CMD_FUNCTION_SET 0x28U
#define LCD_CMD_DISPLAY_ON   0x0CU
#define LCD_CMD_SET_DDRAM    0x80U

#define LCD_INIT_8BIT 0x03U
#define LCD_INIT_4BIT 0x02U

#define LCD_STARTUP_DELAY_MS 50UL
#define LCD_INIT_DELAY1_US   4500UL
#define LCD_INIT_DELAY2_US   150UL
#define LCD_INIT_DELAY_MS    1UL

#define LCD_PULSE_DELAY_US   1UL
#define LCD_DATA_DELAY_US    1UL
#define LCD_CLEAR_DELAY_MS   2UL
#define LCD_COMMAND_DELAY_US 50UL

#define LCD_COLUMNS 20U
#define LCD_ROWS    4U

#define LCD_FIRST_POSITION 1U

#define LCD_HIGH_NIBBLE_SHIFT 4U

static const uint8_t LCD_ADRESSE_LIGNE[LCD_ROWS] = {0x00U, 0x40U, 0x14U, 0x54U};

static void LCD_Delai_Us(uint32_t microsecondes)
{
    uint32_t cyclesRestants;
    uint32_t cyclesAAttendre;
    uint32_t depart;
    uint32_t actuel;
    uint32_t cyclesEcoules;

    cyclesRestants = (CPU_HZ / US_PER_SECOND) * microsecondes;

    while (cyclesRestants > 0UL)
    {
        cyclesAAttendre = cyclesRestants;
        if (cyclesAAttendre > SYSTICK_DELAI_MAX_CYCLES)
        {
            cyclesAAttendre = SYSTICK_DELAI_MAX_CYCLES;
        }

        depart = SYST_CVR;

        /* Le délai reste sous la moitié d'une période SysTick. */
        do
        {
            actuel = SYST_CVR;
            if (depart >= actuel)
            {
                cyclesEcoules = depart - actuel;
            }
            else
            {
                cyclesEcoules = depart + SYSTICK_PERIODE - actuel;
            }
        }
        while (cyclesEcoules < cyclesAAttendre);

        cyclesRestants -= cyclesAAttendre;
    }
}

static void LCD_Delai_Ms(uint32_t millisecondes)
{
    while (millisecondes > 0UL)
    {
        LCD_Delai_Us(US_PER_MS);
        millisecondes--;
    }
}

static void LCD_SetPin(uint8_t broche, bool niveau)
{
    if (niveau)
    {
        GPIOB_BSRR = (1UL << broche); // met la sortie à 1
    }
    else
    {
        GPIOB_BSRR = (1UL << (broche + GPIO_RESET_SHIFT)); // met la sortie à 0
    }
}

static void LCD_Impulsion_E(void)
{
    LCD_SetPin(LCD_E_PIN, true);
    LCD_Delai_Us(LCD_PULSE_DELAY_US);
    LCD_SetPin(LCD_E_PIN, false);
    LCD_Delai_Us(LCD_PULSE_DELAY_US);
}

static void LCD_Envoyer_Nibble(uint8_t nibble)
{
    uint32_t sorties;
    uint32_t remisesAZero;

    nibble &= LCD_DATA_MASK;
    sorties = (uint32_t)nibble << LCD_D4_PIN;
    remisesAZero = (uint32_t)((~nibble) & LCD_DATA_MASK)
                 << (LCD_D4_PIN + GPIO_RESET_SHIFT);
    GPIOB_BSRR = sorties | remisesAZero;

    LCD_Delai_Us(LCD_DATA_DELAY_US); // temps de préparation des données
    LCD_Impulsion_E();
}

static void LCD_GPIO_Init(void)
{
    RCC_APB2ENR |= GPIOB_CLOCK_ENABLE; // active l'horloge de GPIOB

    /* PB9 à PB15: sortie générale push-pull à 2 MHz (MODE=10, CNF=00). */
    GPIOB_CRH &= ~GPIOB_CONFIG_MASK;
    GPIOB_CRH |= GPIOB_CONFIG_OUTPUT;

    /* RS, R/W, E et DB4-DB7 commencent à zéro. */
    GPIOB_BSRR = LCD_7_PINS_MASK << GPIO_RESET_SHIFT;
}

void LCD_Init(void)
{
    LCD_GPIO_Init();

    LCD_Delai_Ms(LCD_STARTUP_DELAY_MS); // laisse le temps au LCD de démarrer

    /* Séquence spéciale: au démarrage, le contrôleur attend encore 8 bits. */
    LCD_Envoyer_Nibble(LCD_INIT_8BIT);
    LCD_Delai_Us(LCD_INIT_DELAY1_US);

    LCD_Envoyer_Nibble(LCD_INIT_8BIT);
    LCD_Delai_Ms(LCD_INIT_DELAY_MS);

    LCD_Envoyer_Nibble(LCD_INIT_8BIT);
    LCD_Delai_Us(LCD_INIT_DELAY2_US);

    LCD_Envoyer_Nibble(LCD_INIT_4BIT); // passe en mode 4 bits
    LCD_Delai_Us(LCD_INIT_DELAY2_US);

    LCD_Commande(false, false, LCD_CMD_FUNCTION_SET); // 4 bits, 2 lignes, 5x8
    LCD_Commande(false, false, LCD_CMD_DISPLAY_OFF);
    LCD_Commande(false, false, LCD_CMD_CLEAR);
    LCD_Commande(false, false, LCD_CMD_ENTRY_MODE);
    LCD_Commande(false, false, LCD_CMD_DISPLAY_ON);
}

void LCD_Commande(bool RW, bool RS, uint8_t Data)
{
    /* L'interface demandée envoie des octets; la lecture n'a pas de retour. */
    if (RW)
    {
        return;
    }

    LCD_SetPin(LCD_RS_PIN, RS);
    LCD_SetPin(LCD_RW_PIN, false); // mode écriture

    LCD_Envoyer_Nibble((uint8_t)(Data >> LCD_HIGH_NIBBLE_SHIFT)); // nibble haut en premier
    LCD_Envoyer_Nibble((uint8_t)(Data & LCD_DATA_MASK)); // puis nibble bas

    if ((!RS) && ((Data == LCD_CMD_CLEAR) || (Data == LCD_CMD_HOME)))
    {
        LCD_Delai_Ms(LCD_CLEAR_DELAY_MS); // ces deux commandes demandent plus de temps
    }
    else
    {
        LCD_Delai_Us(LCD_COMMAND_DELAY_US);
    }
}

void LCD_Ecrire_Char(unsigned char caractere, uint8_t ligne, uint8_t colonne)
{
    uint8_t adresse;


    if ((ligne < LCD_FIRST_POSITION) || (ligne > LCD_ROWS) // 
        || (colonne < LCD_FIRST_POSITION) || (colonne > LCD_COLUMNS)) 
    {
        return;
    }
adresse = (uint8_t)(LCD_CMD_SET_DDRAM
                  | (LCD_ADRESSE_LIGNE[ligne - LCD_FIRST_POSITION]
                  + (uint8_t)(colonne - LCD_FIRST_POSITION)));
    LCD_Commande(false, false, adresse); // place le curseur
    LCD_Commande(false, true, (uint8_t)caractere); // envoie le caractère
}

void LCD_Ecrire_Texte(const char *texte, uint8_t ligne, uint8_t colonne)
{
    while (*texte != '\0')
    {
        LCD_Ecrire_Char((unsigned char)*texte, ligne, colonne);
        texte++;
        colonne++;
    }
}

