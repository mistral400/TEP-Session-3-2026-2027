/**
 ******************************************************************************
 * @file    Lab1.c
 * @brief   Gestion GPIO bare-metal STM32F103
 ******************************************************************************
 */

#include "Lab1.h"


/* ============================================================
 * Adresses de base
 * ============================================================ */

#define RCC_BASE        0x40021000UL
#define AFIO_BASE       0x40010000UL

#define GPIOA_BASE      0x40010800UL
#define GPIOB_BASE      0x40010C00UL


/* ============================================================
 * RCC
 * ============================================================ */

#define RCC_APB2ENR     (*(volatile uint32_t *)(RCC_BASE + 0x18UL))

#define RCC_AFIOEN      (1UL << 0)
#define RCC_IOPAEN      (1UL << 2)
#define RCC_IOPBEN      (1UL << 3)


/* ============================================================
 * AFIO
 * ============================================================ */

#define AFIO_MAPR       (*(volatile uint32_t *)(AFIO_BASE + 0x04UL))

/*
 * SWJ_CFG = 010
 *
 * JTAG désactivé
 * SWD conservé
 *
 * Libère notamment PB3 et PB4.
 */
#define AFIO_SWJ_MASK       (7UL << 24)
#define AFIO_SWJ_SWD_ONLY   (2UL << 24)


/* ============================================================
 * Offsets GPIO STM32F1
 * ============================================================ */

#define GPIO_CRL_OFFSET     0x00UL
#define GPIO_CRH_OFFSET     0x04UL
#define GPIO_IDR_OFFSET     0x08UL
#define GPIO_ODR_OFFSET     0x0CUL
#define GPIO_BSRR_OFFSET    0x10UL
#define GPIO_BRR_OFFSET     0x14UL


/* ============================================================
 * Fonctions internes
 * ============================================================ */

static uint32_t GPIO_GetBase(uint8_t port)
{
    if (port == GPIO_PORT_A)
    {
        return GPIOA_BASE;
    }

    if (port == GPIO_PORT_B)
    {
        return GPIOB_BASE;
    }

    return 0UL;
}


static void GPIO_EnableClock(uint8_t port)
{
    if (port == GPIO_PORT_A)
    {
        RCC_APB2ENR |= RCC_IOPAEN;
    }
    else if (port == GPIO_PORT_B)
    {
        RCC_APB2ENR |= RCC_IOPBEN;
    }
}


/*
 * Libère PB3 et PB4 du port JTAG.
 *
 * SWD reste fonctionnel :
 * PA13 = SWDIO
 * PA14 = SWCLK
 */
static void GPIO_DisableJTAG(void)
{
    RCC_APB2ENR |= RCC_AFIOEN;

    AFIO_MAPR &= ~AFIO_SWJ_MASK;
    AFIO_MAPR |= AFIO_SWJ_SWD_ONLY;
}


/* ============================================================
 * GPIO_InitPin
 * ============================================================ */

void GPIO_InitPin(uint8_t port, uint8_t pin, uint8_t mode)
{
    uint32_t base;
    volatile uint32_t *config_register;

    uint32_t shift;


    if (pin > 15U)
    {
        return;
    }


    base = GPIO_GetBase(port);

    if (base == 0UL)
    {
        return;
    }


    GPIO_EnableClock(port);


    /*
     * Ton Port 2 utilise PB3 et PB4.
     *
     * JTAG doit donc être désactivé.
     */
    if ((port == GPIO_PORT_B) &&
        ((pin == 3U) || (pin == 4U)))
    {
        GPIO_DisableJTAG();
    }


    /*
     * Pins 0 à 7 :
     * CRL
     *
     * Pins 8 à 15 :
     * CRH
     */
    if (pin < 8U)
    {
        config_register =
            (volatile uint32_t *)(base + GPIO_CRL_OFFSET);

        shift = ((uint32_t)pin * 4UL);
    }
    else
    {
        config_register =
            (volatile uint32_t *)(base + GPIO_CRH_OFFSET);

        shift = ((uint32_t)(pin - 8U) * 4UL);
    }


    /*
     * Efface les 4 bits associés à la pin.
     */
    *config_register &= ~(0xFUL << shift);


    if (mode == GPIO_OUTPUT)
    {
        /*
         * STM32F103 :
         *
         * CNF  = 00
         * MODE = 10
         *
         * General purpose output
         * Push-pull
         * 2 MHz
         *
         * 0010 = 0x2
         */
        *config_register |= (0x2UL << shift);
    }
    else
    {
        /*
         * CNF  = 01
         * MODE = 00
         *
         * Floating input
         *
         * 0100 = 0x4
         */
        *config_register |= (0x4UL << shift);
    }
}


/* ============================================================
 * GPIO_WritePin
 * ============================================================ */

void GPIO_WritePin(uint8_t port, uint8_t pin, uint8_t state)
{
    uint32_t base;

    volatile uint32_t *bsrr;
    volatile uint32_t *brr;


    if (pin > 15U)
    {
        return;
    }


    base = GPIO_GetBase(port);

    if (base == 0UL)
    {
        return;
    }


    bsrr = (volatile uint32_t *)(base + GPIO_BSRR_OFFSET);
    brr  = (volatile uint32_t *)(base + GPIO_BRR_OFFSET);


    if (state != 0U)
    {
        /*
         * Met la pin à HIGH.
         */
        *bsrr = (1UL << pin);
    }
    else
    {
        /*
         * Met la pin à LOW.
         */
        *brr = (1UL << pin);
    }
}


/* ============================================================
 * GPIO_ReadPin
 * ============================================================ */

uint8_t GPIO_ReadPin(uint8_t port, uint8_t pin)
{
    uint32_t base;

    volatile uint32_t *idr;


    if (pin > 15U)
    {
        return 0U;
    }


    base = GPIO_GetBase(port);

    if (base == 0UL)
    {
        return 0U;
    }


    idr = (volatile uint32_t *)(base + GPIO_IDR_OFFSET);


    if ((*idr & (1UL << pin)) != 0UL)
    {
        return 1U;
    }

    return 0U;
}


/* ============================================================
 * Systeme_Delai
 * ============================================================ */

void Systeme_Delai(uint32_t delay_ms)
{
    volatile uint32_t outer;
    volatile uint32_t inner;


    for (outer = 0U; outer < delay_ms; outer++)
    {
        for (inner = 0U; inner < 1000U; inner++)
        {
            /*
             * Attente active volontaire.
             */
        }
    }
}