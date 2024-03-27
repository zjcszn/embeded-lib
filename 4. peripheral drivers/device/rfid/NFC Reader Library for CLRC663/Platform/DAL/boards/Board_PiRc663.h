/*----------------------------------------------------------------------------*/
/* Copyright 2017-2021 NXP                                                    */
/*                                                                            */
/* NXP Confidential. This software is owned or controlled by NXP and may only */
/* be used strictly in accordance with the applicable license terms.          */
/* By expressly accepting such terms or by downloading, installing,           */
/* activating and/or otherwise using the software, you are agreeing that you  */
/* have read, and that you agree to comply with and are bound by, such        */
/* license terms. If you do not agree to be bound by the applicable license   */
/* terms, then you may not retain, install, activate or otherwise use the     */
/* software.                                                                  */
/*----------------------------------------------------------------------------*/

/** \file
* Generic phDriver Component of Reader Library Framework.
* $Author$
* $Revision$
* $Date$
*
*/


#ifndef BOARD_PIRC663_H
#define BOARD_PIRC663_H


/*******************************************************************
 *  Enable User space SPI.
 ******************************************************************/
#define PHDRIVER_LINUX_USER_SPI    /**< User Space Spi. Kernel space is supported for PN5180 only. */

/******************************************************************
 * Board Pin/Gpio configurations
 ******************************************************************/
#define PHDRIVER_PIN_RESET          24   /**< "/sys/class/gpio/gpio24/" */
#define PHDRIVER_PIN_IRQ            23  /**< "/sys/class/gpio/gpio23/" */
#define PHDRIVER_PIN_IFSEL0         27
#define PHDRIVER_PIN_IFSEL1         22
#define PHDRIVER_PIN_AD0            4
#define PHDRIVER_PIN_AD1            17

/******************************************************************
 * PIN Pull-Up/Pull-Down configurations.
 ******************************************************************/
#define PHDRIVER_PIN_RESET_PULL_CFG    PH_DRIVER_PULL_DOWN
#define PHDRIVER_PIN_IRQ_PULL_CFG      PH_DRIVER_PULL_UP

/******************************************************************
 * IRQ & BUSY PIN TRIGGER settings
 ******************************************************************/
#define PIN_IRQ_TRIGGER_TYPE         PH_DRIVER_INTERRUPT_RISINGEDGE

/*****************************************************************
 * Front End Reset logic level settings
 ****************************************************************/
#define PH_DRIVER_SET_HIGH            1          /**< Logic High. */
#define PH_DRIVER_SET_LOW             0          /**< Logic Low. */
#define RESET_POWERDOWN_LEVEL       PH_DRIVER_SET_HIGH
#define RESET_POWERUP_LEVEL         PH_DRIVER_SET_LOW

/*****************************************************************
 * SPI Configuration
 ****************************************************************/
#ifdef PHDRIVER_LINUX_USER_SPI
#    define PHDRIVER_USER_SPI_BUS                     0   /**< "/dev/spidev0.0" */
#    define PHDRIVER_USER_SPI_CS                      0   /**< "/dev/spidev0.0" */
#    define PHDRIVER_USER_SPI_FREQ                    5000000 /**< 5 MHz. */
#    define PHDRIVER_USER_SPI_CFG_DIR                "/dev/spidev"
#    define PHDRIVER_USER_SPI_CFG_MODE               SPI_MODE_0
#    define PHDRIVER_USER_SPI_CFG_BITS_PER_WORD      8

/*****************************************************************
 * Dummy entries
 * No functionality. To suppress build error in HAL. No pin functionality in SPI Linux BAL.
 *****************************************************************/
#    define PHDRIVER_PIN_SSEL                            0xFFFF
#    define PHDRIVER_PIN_NSS_PULL_CFG                    PH_DRIVER_PULL_UP

#endif

/*****************************************************************
 * STATUS LED Configuration
 ****************************************************************/
#define PHDRIVER_LED_SUCCESS_DELAY      2

#define PHDRIVER_LED_FAILURE_DELAY_MS   250
#define PHDRIVER_LED_FAILURE_FLICKER    4

#endif /* BOARD_PIRC663_H */
