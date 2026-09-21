#ifndef LAB1_H
#define LAB1_H

#include <stdint.h>

/* ============================================================
 * Ports GPIO supportés
 * ============================================================ */

#define GPIO_PORT_A    0U
#define GPIO_PORT_B    1U

/* ============================================================
 * Modes GPIO
 * ============================================================ */

#define GPIO_INPUT     0U
#define GPIO_OUTPUT    1U


/* ============================================================
 * API publique
 * ============================================================ */

/*
 * Configure une broche GPIO.
 *
 * port :
 *     GPIO_PORT_A
 *     GPIO_PORT_B
 *
 * pin :
 *     0 à 15
 *
 * mode :
 *     GPIO_INPUT
 *     GPIO_OUTPUT
 */
void GPIO_InitPin(uint8_t port, uint8_t pin, uint8_t mode);


/*
 * Écrit un niveau logique sur une sortie GPIO.
 */
void GPIO_WritePin(uint8_t port, uint8_t pin, uint8_t state);


/*
 * Lit l'état logique d'une broche GPIO.
 *
 * Retour :
 *     0 = LOW
 *     1 = HIGH
 */
uint8_t GPIO_ReadPin(uint8_t port, uint8_t pin);


/*
 * Produit un délai logiciel approximatif en millisecondes.
 */
void Systeme_Delai(uint32_t delay_ms);

#endif