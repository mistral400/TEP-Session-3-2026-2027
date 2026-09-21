#include "lcd.h"

/*
 * Pilote HD44780 20 x 4, bus 4 bits, STM32F103 à 8 MHz.
 * Aucun HAL/LL : tous les accès matériels se font directement par registres.
 */

// !! CHANGEMENT : le câblage existant est conservé et reste privé à ce fichier.
#define LCD_RS_PIN                 9U
#define LCD_RW_PIN                10U
#define LCD_EN_PIN                11U
#define LCD_D4_PIN                12U
#define LCD_D5_PIN                13U
#define LCD_D6_PIN                14U
#define LCD_D7_PIN                15U

#define LCD_COLUMNS               20U
#define LCD_ROWS                  4U

#define RCC_BASE                  0x40021000UL
#define GPIOA_BASE                0x40010800UL
#define GPIOB_BASE                0x40010C00UL
#define GPIO_PORT_SPACING         0x00000400UL
#define GPIO_CRH_OFFSET           0x04UL
#define GPIO_ODR_OFFSET           0x0CUL
#define RCC_APB2ENR_OFFSET        0x18UL

#define SYSTICK_BASE              0xE000E010UL
#define SYSTICK_CTRL_OFFSET       0x00UL
#define SYSTICK_LOAD_OFFSET       0x04UL
#define SYSTICK_VAL_OFFSET        0x08UL

#define REG32(address)            (*(volatile uint32_t *)(address))
#define RCC_APB2ENR               REG32(RCC_BASE + RCC_APB2ENR_OFFSET)
#define GPIOB_CRH                 REG32(GPIOB_BASE + GPIO_CRH_OFFSET)
#define SYSTICK_CTRL              REG32(SYSTICK_BASE + SYSTICK_CTRL_OFFSET)
#define SYSTICK_LOAD              REG32(SYSTICK_BASE + SYSTICK_LOAD_OFFSET)
#define SYSTICK_VAL               REG32(SYSTICK_BASE + SYSTICK_VAL_OFFSET)

#define RCC_IOPBEN                (1UL << 3)
#define SYSTICK_ENABLE            (1UL << 0)
#define SYSTICK_CLKSOURCE_CPU     (1UL << 2)
#define SYSTICK_COUNTFLAG         (1UL << 16)
#define SYSTICK_TICKS_PER_MS      8000UL

#define LCD_CMD_CLEAR             0x01U
#define LCD_CMD_HOME              0x02U
#define LCD_CMD_ENTRY_MODE        0x06U
#define LCD_CMD_DISPLAY_CONTROL   0x08U
#define LCD_CMD_FUNCTION_SET      0x28U
#define LCD_CMD_SHIFT_LEFT        0x18U
#define LCD_CMD_SHIFT_RIGHT       0x1CU
#define LCD_CMD_SET_CGRAM         0x40U
#define LCD_CMD_SET_DDRAM         0x80U

#define LCD_DISPLAY_BIT           0x04U
#define LCD_CURSOR_BIT            0x02U
#define LCD_BLINK_BIT             0x01U
#define LCD_FULL_BLOCK            0xFFU

/* PB est le port numéro 1 par rapport à l'adresse de GPIOA. */
#define LCD_PORT_INDEX            1U

static const uint8_t lcd_row_address[LCD_ROWS] = {0x00U, 0x40U, 0x14U, 0x54U};

// !! CHANGEMENT : état minimal mémorisé pour sécuriser l'API haut niveau.
static uint8_t lcd_current_col = 0U;
static uint8_t lcd_current_row = 0U;
static bool lcd_display_enabled = true;
static bool lcd_cursor_visible = false;
static bool lcd_blink_enabled = false;

/* Fonctions matérielles privées. */
static void LCD_GPIO_Init(void);
static void LCD_SysTickInit(void);
static void LCD_Delay(uint16_t time_ms);
static void LCD_WriteToPin(uint8_t port_index, uint8_t pin, bool value);
static void LCD_EnablePulse(void);
static void LCD_Send4Bits(uint8_t data);
static void LCD_WriteCommandRaw(uint8_t command);
static void LCD_WriteDataRaw(uint8_t data);

/* Fonctions de service privées de l'API haut niveau. */
static bool LCD_PositionIsValid(uint8_t col, uint8_t row);
static void LCD_ApplyDisplayControl(void);
static void LCD_TrackCommand(uint8_t command);
static void LCD_RestoreCursor(uint8_t col, uint8_t row);
static void LCD_WriteUnsignedBase(uint32_t value, uint8_t base);

/* Configure PB9..PB15 en sorties push-pull à 10 MHz, comme le code d'origine. */
static void LCD_GPIO_Init(void)
{
    const uint32_t configuration_mask = 0xFFFFFFF0UL;

    RCC_APB2ENR |= RCC_IOPBEN;
    GPIOB_CRH &= ~configuration_mask;
    GPIOB_CRH |= 0x11111110UL;
}

/* Active SysTick avec l'horloge processeur (8 MHz dans le projet actuel). */
static void LCD_SysTickInit(void)
{
    SYSTICK_CTRL = 0U;
    SYSTICK_LOAD = SYSTICK_TICKS_PER_MS - 1U;
    SYSTICK_VAL = 0U;
    SYSTICK_CTRL = SYSTICK_ENABLE | SYSTICK_CLKSOURCE_CPU;
}

/*
 * Attend un nombre entier de millisecondes.
 * VAL est remis à zéro à chaque milliseconde. L'ancien code
 * modifiait seulement LOAD, donc la première attente dépendait du comptage déjà
 * en cours et pouvait être plus courte que demandé.
 */
// !! CHANGEMENT : temporisation rendue déterministe par remise à zéro de VAL.
static void LCD_Delay(uint16_t time_ms)
{
    while (time_ms > 0U)
    {
        SYSTICK_LOAD = SYSTICK_TICKS_PER_MS - 1U;
        SYSTICK_VAL = 0U;

        while ((SYSTICK_CTRL & SYSTICK_COUNTFLAG) == 0U)
        {
            /* Attente active volontaire : pilote bare-metal bloquant. */
        }

        --time_ms;
    }
}

/* Écrit une valeur dans une broche GPIO par accès direct au registre ODR. */
static void LCD_WriteToPin(uint8_t port_index, uint8_t pin, bool value)
{
    volatile uint32_t *odr = (volatile uint32_t *)(GPIOA_BASE
                                                   + ((uint32_t)port_index * GPIO_PORT_SPACING)
                                                   + GPIO_ODR_OFFSET);
    const uint32_t mask = 1UL << pin;

    if (value)
    {
        *odr |= mask;
    }
    else
    {
        *odr &= ~mask;
    }
}

/* Produit le front haut puis bas requis sur la broche Enable. */
static void LCD_EnablePulse(void)
{
    LCD_WriteToPin(LCD_PORT_INDEX, LCD_EN_PIN, true);
    LCD_Delay(1U);
    LCD_WriteToPin(LCD_PORT_INDEX, LCD_EN_PIN, false);
}

/* Place un nibble sur D4..D7 puis le valide avec Enable. */
static void LCD_Send4Bits(uint8_t data)
{
    LCD_WriteToPin(LCD_PORT_INDEX, LCD_D4_PIN, (data & 0x01U) != 0U);
    LCD_WriteToPin(LCD_PORT_INDEX, LCD_D5_PIN, (data & 0x02U) != 0U);
    LCD_WriteToPin(LCD_PORT_INDEX, LCD_D6_PIN, (data & 0x04U) != 0U);
    LCD_WriteToPin(LCD_PORT_INDEX, LCD_D7_PIN, (data & 0x08U) != 0U);
    LCD_EnablePulse();
}

/* Envoie une commande complète en deux nibbles, sans modifier l'état logiciel. */
static void LCD_WriteCommandRaw(uint8_t command)
{
    LCD_WriteToPin(LCD_PORT_INDEX, LCD_RS_PIN, false);
    LCD_WriteToPin(LCD_PORT_INDEX, LCD_RW_PIN, false);
    LCD_Send4Bits((uint8_t)(command >> 4));
    LCD_Send4Bits((uint8_t)(command & 0x0FU));
}

/* Envoie une donnée complète en deux nibbles. */
static void LCD_WriteDataRaw(uint8_t data)
{
    LCD_WriteToPin(LCD_PORT_INDEX, LCD_RS_PIN, true);
    LCD_WriteToPin(LCD_PORT_INDEX, LCD_RW_PIN, false);
    LCD_Send4Bits((uint8_t)(data >> 4));
    LCD_Send4Bits((uint8_t)(data & 0x0FU));
}

static bool LCD_PositionIsValid(uint8_t col, uint8_t row)
{
    return (col < LCD_COLUMNS) && (row < LCD_ROWS);
}

/* Réapplique en une seule commande les trois options d'affichage du HD44780. */
static void LCD_ApplyDisplayControl(void)
{
    uint8_t command = LCD_CMD_DISPLAY_CONTROL;

    if (lcd_display_enabled)
    {
        command |= LCD_DISPLAY_BIT;
    }
    if (lcd_cursor_visible)
    {
        command |= LCD_CURSOR_BIT;
    }
    if (lcd_blink_enabled)
    {
        command |= LCD_BLINK_BIT;
    }

    LCD_WriteCommandRaw(command);
}

/* Met à jour l'état connu après une commande historique envoyée par l'appelant. */
static void LCD_TrackCommand(uint8_t command)
{
    uint8_t row;

    if (command == LCD_CMD_CLEAR || command == LCD_CMD_HOME)
    {
        lcd_current_col = 0U;
        lcd_current_row = 0U;
    }
    else if ((command & LCD_CMD_SET_DDRAM) != 0U)
    {
        const uint8_t address = command & 0x7FU;

        for (row = 0U; row < LCD_ROWS; ++row)
        {
            if ((address >= lcd_row_address[row])
                && (address < (uint8_t)(lcd_row_address[row] + LCD_COLUMNS)))
            {
                lcd_current_row = row;
                lcd_current_col = (uint8_t)(address - lcd_row_address[row]);
                break;
            }
        }
    }
    else if ((command & 0xF8U) == LCD_CMD_DISPLAY_CONTROL)
    {
        lcd_display_enabled = (command & LCD_DISPLAY_BIT) != 0U;
        lcd_cursor_visible = (command & LCD_CURSOR_BIT) != 0U;
        lcd_blink_enabled = (command & LCD_BLINK_BIT) != 0U;
    }
}

/* Replace le contrôleur après un accès temporaire à la CGRAM. */
static void LCD_RestoreCursor(uint8_t col, uint8_t row)
{
    uint8_t hardware_col;

    if (row >= LCD_ROWS)
    {
        row = 0U;
    }

    hardware_col = (col < LCD_COLUMNS) ? col : (LCD_COLUMNS - 1U);
    LCD_WriteCommandRaw((uint8_t)(LCD_CMD_SET_DDRAM
                                  | (lcd_row_address[row] + hardware_col)));
    lcd_current_col = col;
    lcd_current_row = row;
}

/* Convertit un entier non signé dans une base 2..16, sans sprintf. */
static void LCD_WriteUnsignedBase(uint32_t value, uint8_t base)
{
    static const char digits[] = "0123456789ABCDEF";
    char buffer[33];
    uint8_t index = (uint8_t)sizeof(buffer);

    if (base < 2U || base > 16U)
    {
        return;
    }

    do
    {
        const uint32_t digit = value % base;
        --index;
        buffer[index] = digits[digit];
        value /= base;
    } while (value != 0U);

    while (index < (uint8_t)sizeof(buffer))
    {
        LCD_SendChar(buffer[index]);
        ++index;
    }
}

/* Initialise l'afficheur en mode 4 bits avec le câblage historique. */
void LCD_Init(void)
{
    LCD_GPIO_Init();
    LCD_SysTickInit();
    LCD_Delay(50U);

    /* Séquence d'initialisation 4 bits conservée depuis le pilote existant. */
    LCD_WriteCommandRaw(0x33U);
    LCD_WriteCommandRaw(0x32U);
    LCD_WriteCommandRaw(LCD_CMD_FUNCTION_SET);

    lcd_display_enabled = true;
    lcd_cursor_visible = false;
    lcd_blink_enabled = false;
    LCD_ApplyDisplayControl();

    LCD_WriteCommandRaw(LCD_CMD_ENTRY_MODE);
    LCD_Clear();
}

/* Efface tout l'écran et replace le curseur en haut à gauche. */
void LCD_Clear(void)
{
    LCD_WriteCommandRaw(LCD_CMD_CLEAR);
    LCD_Delay(2U);
    lcd_current_col = 0U;
    lcd_current_row = 0U;
}

// !! CHANGEMENT : replace le curseur en (0, 0) sans effacer le contenu.
void LCD_Home(void)
{
    LCD_WriteCommandRaw(LCD_CMD_HOME);
    LCD_Delay(2U);
    lcd_current_col = 0U;
    lcd_current_row = 0U;
}

// !! CHANGEMENT : rend visibles les caractères déjà présents en DDRAM.
void LCD_DisplayOn(void)
{
    lcd_display_enabled = true;
    LCD_ApplyDisplayControl();
}

// !! CHANGEMENT : masque l'écran sans effacer son contenu.
void LCD_DisplayOff(void)
{
    lcd_display_enabled = false;
    LCD_ApplyDisplayControl();
}

// !! CHANGEMENT : active ou masque le curseur, sans changer les autres options.
void LCD_SetCursorVisible(bool enabled)
{
    lcd_cursor_visible = enabled;
    LCD_ApplyDisplayControl();
}

// !! CHANGEMENT : active ou désactive le clignotement du curseur.
void LCD_SetBlink(bool enabled)
{
    lcd_blink_enabled = enabled;
    LCD_ApplyDisplayControl();
}

// !! CHANGEMENT : décale tout l'affichage d'une position vers la gauche.
void LCD_ShiftLeft(void)
{
    LCD_WriteCommandRaw(LCD_CMD_SHIFT_LEFT);
}

// !! CHANGEMENT : décale tout l'affichage d'une position vers la droite.
void LCD_ShiftRight(void)
{
    LCD_WriteCommandRaw(LCD_CMD_SHIFT_RIGHT);
}

/*
 * Écrit depuis la position courante. En fin de ligne, l'écriture
 * continue au début de la ligne logique suivante et s'arrête après la ligne 3.
 */
// !! CHANGEMENT : nouvelle écriture bornée avec passage logique à la ligne.
void LCD_Write(const char *text)
{
    if (text == 0)
    {
        return;
    }

    while (*text != '\0')
    {
        if (*text == '\n')
        {
            if ((uint8_t)(lcd_current_row + 1U) >= LCD_ROWS)
            {
                return;
            }
            LCD_SetCursor(0U, (uint8_t)(lcd_current_row + 1U));
            ++text;
            continue;
        }

        if (lcd_current_col >= LCD_COLUMNS)
        {
            if ((uint8_t)(lcd_current_row + 1U) >= LCD_ROWS)
            {
                return;
            }
            LCD_SetCursor(0U, (uint8_t)(lcd_current_row + 1U));
        }

        LCD_SendChar(*text);
        ++text;
    }
}

// !! CHANGEMENT : écrit à une position valide et tronque au bord droit.
void LCD_WriteAt(uint8_t col, uint8_t row, const char *text)
{
    if (!LCD_PositionIsValid(col, row) || text == 0)
    {
        return;
    }

    LCD_SetCursor(col, row);
    while (*text != '\0' && *text != '\n' && lcd_current_col < LCD_COLUMNS)
    {
        LCD_SendChar(*text);
        ++text;
    }
}

// !! CHANGEMENT : remplace une ligne entière et complète la fin avec des espaces.
void LCD_WriteLine(uint8_t row, const char *text)
{
    uint8_t col = 0U;

    if (row >= LCD_ROWS || text == 0)
    {
        return;
    }

    LCD_SetCursor(0U, row);
    while (col < LCD_COLUMNS && text[col] != '\0' && text[col] != '\n')
    {
        LCD_SendChar(text[col]);
        ++col;
    }
    while (col < LCD_COLUMNS)
    {
        LCD_SendChar(' ');
        ++col;
    }
}

// !! CHANGEMENT : efface uniquement la ligne demandée puis revient à sa colonne 0.
void LCD_ClearLine(uint8_t row)
{
    uint8_t col;

    if (row >= LCD_ROWS)
    {
        return;
    }

    LCD_SetCursor(0U, row);
    for (col = 0U; col < LCD_COLUMNS; ++col)
    {
        LCD_SendChar(' ');
    }
    LCD_SetCursor(0U, row);
}

// !! CHANGEMENT : centre un texte tronqué à 20 caractères sur une ligne nettoyée.
void LCD_CenterText(uint8_t row, const char *text)
{
    uint8_t length = 0U;
    uint8_t col;
    uint8_t left_padding;

    if (row >= LCD_ROWS || text == 0)
    {
        return;
    }

    while (length < LCD_COLUMNS && text[length] != '\0' && text[length] != '\n')
    {
        ++length;
    }
    left_padding = (uint8_t)((LCD_COLUMNS - length) / 2U);

    LCD_SetCursor(0U, row);
    for (col = 0U; col < left_padding; ++col)
    {
        LCD_SendChar(' ');
    }
    for (col = 0U; col < length; ++col)
    {
        LCD_SendChar(text[col]);
    }
    while (lcd_current_col < LCD_COLUMNS)
    {
        LCD_SendChar(' ');
    }
}

// !! CHANGEMENT : affiche un entier signé décimal, y compris INT32_MIN.
void LCD_PrintInt(int32_t value)
{
    uint32_t magnitude;

    if (value < 0)
    {
        LCD_SendChar('-');
        magnitude = (uint32_t)(-(value + 1)) + 1U;
    }
    else
    {
        magnitude = (uint32_t)value;
    }

    LCD_WriteUnsignedBase(magnitude, 10U);
}

// !! CHANGEMENT : affiche un entier non signé en décimal.
void LCD_PrintUInt(uint32_t value)
{
    LCD_WriteUnsignedBase(value, 10U);
}

// !! CHANGEMENT : affiche un entier en hexadécimal majuscule, sans préfixe 0x.
void LCD_PrintHex(uint32_t value)
{
    LCD_WriteUnsignedBase(value, 16U);
}

// !! CHANGEMENT : affiche un entier en binaire, sans préfixe 0b ni zéros initiaux.
void LCD_PrintBinary(uint32_t value)
{
    LCD_WriteUnsignedBase(value, 2U);
}

// !! CHANGEMENT : positionne le curseur puis affiche un entier signé.
void LCD_PrintIntAt(uint8_t col, uint8_t row, int32_t value)
{
    if (!LCD_PositionIsValid(col, row))
    {
        return;
    }

    LCD_SetCursor(col, row);
    LCD_PrintInt(value);
}

/*
 * Programme l'un des 8 caractères de la CGRAM.
 * Seuls les 5 bits de poids faible de chaque ligne sont utilisés par le LCD.
 */
// !! CHANGEMENT : ajout de la programmation CGRAM avec restauration du curseur.
void LCD_CreateChar(uint8_t slot, const uint8_t pattern[8])
{
    uint8_t index;
    const uint8_t saved_col = lcd_current_col;
    const uint8_t saved_row = lcd_current_row;

    if (slot >= 8U || pattern == 0)
    {
        return;
    }

    LCD_WriteCommandRaw((uint8_t)(LCD_CMD_SET_CGRAM | (slot << 3)));
    for (index = 0U; index < 8U; ++index)
    {
        LCD_WriteDataRaw((uint8_t)(pattern[index] & 0x1FU));
    }
    LCD_RestoreCursor(saved_col, saved_row);
}

// !! CHANGEMENT : affiche un caractère personnalisé préalablement créé (0..7).
void LCD_PrintCustomChar(uint8_t slot)
{
    if (slot >= 8U || lcd_current_col >= LCD_COLUMNS)
    {
        return;
    }

    LCD_WriteDataRaw(slot);
    ++lcd_current_col;
}

/*
 * Dessine une barre sur toute une ligne avec le bloc plein 0xFF.
 * value est limité à max. Si max vaut zéro ou si row est invalide, aucun effet.
 */
// !! CHANGEMENT : ajout de la barre de progression bornée.
void LCD_ProgressBar(uint8_t row, uint8_t value, uint8_t max)
{
    uint8_t col;
    uint8_t filled;

    if (row >= LCD_ROWS || max == 0U)
    {
        return;
    }
    if (value > max)
    {
        value = max;
    }

    filled = (uint8_t)(((uint16_t)value * LCD_COLUMNS) / max);
    LCD_SetCursor(0U, row);
    for (col = 0U; col < LCD_COLUMNS; ++col)
    {
        LCD_WriteDataRaw((col < filled) ? LCD_FULL_BLOCK : (uint8_t)' ');
        ++lcd_current_col;
    }
}

/* API historique : envoie une commande brute tout en suivant les commandes connues. */
void LCD_SendCommand(uint8_t cmd)
{
    LCD_WriteCommandRaw(cmd);
    LCD_TrackCommand(cmd);

    if (cmd == LCD_CMD_CLEAR || cmd == LCD_CMD_HOME)
    {
        LCD_Delay(2U);
    }
}

/* API historique : envoie un caractère à la position courante. */
void LCD_SendChar(char data)
{
    if (lcd_current_col >= LCD_COLUMNS || lcd_current_row >= LCD_ROWS)
    {
        return;
    }

    LCD_WriteDataRaw((uint8_t)data);
    ++lcd_current_col;
}

/* API historique : l'ancienne fonction réutilise maintenant l'écriture sécurisée. */
void LCD_SendString(const char *str)
{
    LCD_Write(str);
}

/* API historique : ignore simplement une coordonnée hors de l'écran 20 x 4. */
void LCD_SetCursor(uint8_t col, uint8_t row)
{
    if (!LCD_PositionIsValid(col, row))
    {
        return;
    }

    LCD_WriteCommandRaw((uint8_t)(LCD_CMD_SET_DDRAM
                                  | (lcd_row_address[row] + col)));
    lcd_current_col = col;
    lcd_current_row = row;
}
