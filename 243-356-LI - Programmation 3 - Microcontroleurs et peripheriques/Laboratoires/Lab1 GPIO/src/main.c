#include "lcd.h"
#include "Lab1.h"

#include <stdint.h>


/* ============================================================
 * Mapping physique du PORT 1
 *
 * 8 boutons
 * ============================================================ */

static const uint8_t port1_pins[8] =
{
    1U,     // P1_0 -> PA1
    2U,     // P1_1 -> PA2
    3U,     // P1_2 -> PA3
    4U,     // P1_3 -> PA4
    5U,     // P1_4 -> PA5
    6U,     // P1_5 -> PA6
    7U,     // P1_6 -> PA7
    8U      // P1_7 -> PA8
};


/* ============================================================
 * Mapping physique du PORT 2
 *
 * 8 LED
 * ============================================================ */

static const uint8_t port2_pins[8] =
{
    0U,     // P2_0 -> PB0
    1U,     // P2_1 -> PB1
    3U,     // P2_2 -> PB3
    4U,     // P2_3 -> PB4
    7U,     // P2_4 -> PB7
    6U,     // P2_5 -> PB6
    5U,     // P2_6 -> PB5
    8U      // P2_7 -> PB8
};


/* ============================================================
 * Affiche l'état d'un bouton sur le LCD
 * ============================================================ */

static void DisplayButtonState(
    uint8_t button_index,
    uint8_t pressed)
{
    uint8_t input_pin;
    uint8_t output_pin;


    input_pin = port1_pins[button_index];
    output_pin = port2_pins[button_index];


    LCD_Clear();


    /*
     * Ligne 0
     *
     * Exemple :
     * P1_3 APPUYE
     */
    LCD_Write("P1_");
    LCD_PrintUInt(button_index);

    if (pressed != 0U)
    {
        LCD_Write(" APPUYE");
    }
    else
    {
        LCD_Write(" RELACHE");
    }


    /*
     * Ligne 1
     *
     * Exemple :
     * Bouton: PA4
     */
    LCD_WriteAt(0U, 1U, "Bouton: PA");
    LCD_PrintUInt(input_pin);


    /*
     * Ligne 2
     *
     * Exemple :
     * Sortie: PB4
     */
    LCD_WriteAt(0U, 2U, "Sortie: PB");
    LCD_PrintUInt(output_pin);


    /*
     * Ligne 3
     *
     * Exemple :
     * P2_3 LED = ON
     */
    LCD_WriteAt(0U, 3U, "P2_");
    LCD_PrintUInt(button_index);

    LCD_Write(" LED = ");

    if (pressed != 0U)
    {
        LCD_Write("ON");
    }
    else
    {
        LCD_Write("OFF");
    }
}


/* ============================================================
 * main
 * ============================================================ */

int main(void)
{
    uint8_t i;

    uint8_t current_state[8];
    uint8_t previous_state[8];


    /* ========================================================
     * Initialisation LCD
     * ======================================================== */

    LCD_Init();


    /* ========================================================
     * PORT 1
     *
     * PA1 -> PA8
     *
     * Entrées boutons
     * ======================================================== */

    for (i = 0U; i < 8U; i++)
    {
        GPIO_InitPin(
            GPIO_PORT_A,
            port1_pins[i],
            GPIO_INPUT
        );


        /*
         * Lit l'état initial.
         */
        previous_state[i] =
            GPIO_ReadPin(
                GPIO_PORT_A,
                port1_pins[i]
            );
    }


    /* ========================================================
     * PORT 2
     *
     * Sorties LED
     * ======================================================== */

    for (i = 0U; i < 8U; i++)
    {
        GPIO_InitPin(
            GPIO_PORT_B,
            port2_pins[i],
            GPIO_OUTPUT
        );


        /*
         * Tes LED semblent être actives LOW.
         *
         * HIGH = OFF
         */
        GPIO_WritePin(
            GPIO_PORT_B,
            port2_pins[i],
            1U
        );
    }


    /* ========================================================
     * Écran de démarrage
     * ======================================================== */

    LCD_Clear();

    LCD_CenterText(0U, "TEST GPIO");

    LCD_WriteLine(
        1U,
        "PORT1 = 8 boutons"
    );

    LCD_WriteLine(
        2U,
        "PORT2 = 8 LED"
    );

    LCD_WriteLine(
        3U,
        "Appuie un bouton"
    );


    /* ========================================================
     * Boucle principale
     * ======================================================== */

    while (1)
    {
        for (i = 0U; i < 8U; i++)
        {
            current_state[i] =
                GPIO_ReadPin(
                    GPIO_PORT_A,
                    port1_pins[i]
                );


            /* =================================================
             * Bouton appuyé
             *
             * HIGH -> LOW
             * ================================================= */

            if ((previous_state[i] == 1U) &&
                (current_state[i] == 0U))
            {
                /*
                 * Anti-rebond.
                 */
                Systeme_Delai(20U);


                /*
                 * Vérification après anti-rebond.
                 */
                if (GPIO_ReadPin(
                        GPIO_PORT_A,
                        port1_pins[i]) == 0U)
                {
                    /*
                     * Allume la LED correspondante.
                     *
                     * LED active LOW.
                     */
                    GPIO_WritePin(
                        GPIO_PORT_B,
                        port2_pins[i],
                        0U
                    );


                    /*
                     * Affichage LCD.
                     */
                    DisplayButtonState(
                        i,
                        1U
                    );
                }
            }


            /* =================================================
             * Bouton relâché
             *
             * LOW -> HIGH
             * ================================================= */

            else if ((previous_state[i] == 0U) &&
                     (current_state[i] == 1U))
            {
                Systeme_Delai(20U);


                if (GPIO_ReadPin(
                        GPIO_PORT_A,
                        port1_pins[i]) == 1U)
                {
                    /*
                     * Éteint la LED correspondante.
                     */
                    GPIO_WritePin(
                        GPIO_PORT_B,
                        port2_pins[i],
                        1U
                    );


                    DisplayButtonState(
                        i,
                        0U
                    );
                }
            }


            /*
             * Sauvegarde l'état.
             */
            previous_state[i] = current_state[i];
        }
    }
}