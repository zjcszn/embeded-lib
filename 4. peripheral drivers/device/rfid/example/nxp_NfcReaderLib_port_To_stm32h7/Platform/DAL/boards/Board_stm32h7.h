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


#ifndef BOARD_STM32H7_H
#define BOARD_STM32H7_H

#define PORTA                      ((unsigned int)('A' - 'A') << 8)          /**< PortA. */
#define PORTB                      ((unsigned int)('B' - 'A') << 8)          /**< PortA. */


/******************************************************************
 * Board Pin/Gpio configurations
 ******************************************************************/
#define PHDRIVER_PIN_RESET         (PORTA | 0UL)    /**< Reset pin, Port A, pin0. */
#define PHDRIVER_PIN_IRQ           (PORTA | 1UL)    /**< Interrupt pin from Frontend to Host, PortA, pin1 */


/******************************************************************
 * PIN Pull-Up/Pull-Down configurations.
 ******************************************************************/
#define PHDRIVER_PIN_RESET_PULL_CFG    PH_DRIVER_PULL_DOWN
#define PHDRIVER_PIN_IRQ_PULL_CFG      PH_DRIVER_PULL_UP
#define PHDRIVER_PIN_NSS_PULL_CFG      PH_DRIVER_PULL_UP

/******************************************************************
 * IRQ PIN NVIC settings
 ******************************************************************/
#define PIN_IRQ_TRIGGER_TYPE        PH_DRIVER_INTERRUPT_FALLINGEDGE  /**< IRQ pin Falling edge interrupt */
#define EINT_PRIORITY               5                /**< Interrupt priority. */
#define EINT_IRQn                   EXTI1_IRQn       /**< NVIC IRQ */
#define EINT_IRQHandler             EXTI1_IRQHandler /**< NVIC IRQ */

/*****************************************************************
 * Front End Reset logic level settings
 ****************************************************************/
#define PH_DRIVER_SET_HIGH            1          /**< Logic High. */
#define PH_DRIVER_SET_LOW             0          /**< Logic Low. */
#define RESET_POWERDOWN_LEVEL         PH_DRIVER_SET_HIGH
#define RESET_POWERUP_LEVEL           PH_DRIVER_SET_LOW

/*****************************************************************
 * SPI Configuration
 ****************************************************************/


/******************************************************************/
/* Pin configuration format : Its a 32 bit format where 1st 3 bytes represents each field as shown below.
 * | Byte3 | Byte2 | Byte1 | Byte0 |
 * | ---   | FUNC  | PORT  | PIN   |
 * */

#define PHDRIVER_PIN_SSEL     (PORTA | 4UL)  /**< Slave select, Port0, pin16, Pin function 0 */


/*****************************************************************
 * Timer Configuration
 ****************************************************************/
#define PH_DRIVER_LPC_TIMER                    LPC_TIMER0           /**< Use LPC timer0 */
#define PH_DRIVER_LPC_TIMER_CLK                SYSCTL_CLOCK_TIMER0  /**< Timer 0 clock source */
#define PH_DRIVER_LPC_TIMER_MATCH_REGISTER     0x01  /* use match register 1 always. */
#define PH_DRIVER_LPC_TIMER_IRQ                TIMER0_IRQn          /**< NVIC Timer0 Irq */
#define PH_DRIVER_LPC_TIMER_IRQ_HANDLER        TIMER0_IRQHandler    /**< Timer0 Irq Handler */
#define PH_DRIVER_LPC_TIMER_IRQ_PRIORITY       5                    /**< NVIC Timer0 Irq priority */

#endif /* BOARD_STM32H7_H */
