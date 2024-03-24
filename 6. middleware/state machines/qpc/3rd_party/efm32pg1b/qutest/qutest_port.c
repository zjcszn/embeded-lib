//============================================================================
// Product: QUTEST port for the EFM32-SLSTK3401A board
// Last updated for version 7.3.2
// Last updated on  2023-12-13
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// Copyright (C) 2005 Quantum Leaps, LLC. <state-machine.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open source GNU
// General Public License version 3 (or any later version), or alternatively,
// under the terms of one of the closed source Quantum Leaps commercial
// licenses.
//
// The terms of the open source GNU General Public License version 3
// can be found at: <www.gnu.org/licenses/gpl-3.0>
//
// The terms of the closed source Quantum Leaps commercial licenses
// can be found at: <www.state-machine.com/licensing>
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// Contact information:
// <www.state-machine.com>
// <info@state-machine.com>
//============================================================================
#ifndef Q_SPY
    #error "Q_SPY must be defined to compile qutest_port.c"
#endif // Q_SPY

#define QP_IMPL        // this is QP implementation
#include "qp_port.h"   // QP port
#include "qsafe.h"     // QP Functional Safety (FuSa) Subsystem
#include "qs_port.h"   // QS port
#include "qs_pkg.h"    // QS package-scope interface

#include "em_device.h" // the device specific header (SiLabs)
#include "em_cmu.h"    // Clock Management Unit (SiLabs)
#include "em_gpio.h"   // GPIO (SiLabs)
#include "em_usart.h"  // USART (SiLabs)
// add other drivers if necessary...

//Q_DEFINE_THIS_MODULE("qutest_port")

// ISRs defined in this BSP --------------------------------------------------
void USART0_RX_IRQHandler(void);

// Local-scope objects -------------------------------------------------------
static USART_TypeDef * const l_USART0 = ((USART_TypeDef *)(0x40010000UL));

#define LED_PORT    gpioPortF
#define LED0_PIN    4
#define LED1_PIN    5

#define PB_PORT     gpioPortF
#define PB0_PIN     6
#define PB1_PIN     7

//............................................................................
//
// ISR for receiving bytes from the QSPY Back-End
// NOTE: This ISR is "QF-unaware" meaning that it does not interact with
// the QK/QXK and is not disabled. Such ISRs don't need to call
// QK/QXK_ISR_EXIT and they cannot post or publish events.
//
void USART0_RX_IRQHandler(void) {
    // while RX FIFO NOT empty
    while ((l_USART0->STATUS & USART_STATUS_RXDATAV) != 0U) {
        uint32_t b = l_USART0->RXDATA;
        QS_RX_PUT(b);
    }
}

//............................................................................
void assert_failed(char const * const module, int_t const id); // prototype
void assert_failed(char const * const module, int_t const id) {
    Q_onError(module, id);
}

// QS callbacks ==============================================================
uint8_t QS_onStartup(void const *arg) {
    Q_UNUSED_PAR(arg);

    static uint8_t qsTxBuf[2*1024]; // buffer for QS-TX channel
    QS_initBuf  (qsTxBuf, sizeof(qsTxBuf));

    static uint8_t qsRxBuf[256];    // buffer for QS-RX channel
    QS_rxInitBuf(qsRxBuf, sizeof(qsRxBuf));

    // Enable peripheral clocks
    CMU_ClockEnable(cmuClock_HFPER, true);
    CMU_ClockEnable(cmuClock_GPIO, true);

    // To avoid false start, configure output as high
    GPIO_PinModeSet(gpioPortA, 0, gpioModePushPull, 1); // TX pin
    GPIO_PinModeSet(gpioPortA, 1, gpioModeInput, 0);    // RX pin

    // Enable DK RS232/UART switch
    GPIO_PinModeSet(gpioPortA, 5, gpioModePushPull, 1);
    CMU_ClockEnable(cmuClock_USART0, true);

    // configure the UART for the desired baud rate, 8-N-1 operation
    static USART_InitAsync_TypeDef init = {
        usartEnable,      // Enable RX/TX when init completed
        0,                // Use current clock for configuring baudrate
        115200,           // 115200 bits/s
        usartOVS16,       // 16x oversampling
        usartDatabits8,   // 8 databits
        usartNoParity,    // No parity
        usartStopbits1,   // 1 stopbit
        0,                // Do not disable majority vote
        0,                // Not USART PRS input mode
        usartPrsRxCh0,    // PRS channel 0
        0,                // Auto CS functionality enable/disable switch
        0,                // Auto CS Hold cycles
        0                 // Auto CS Setup cycles
    };
    init.enable = usartDisable;
    USART_InitAsync(l_USART0, &init);

    // enable pins at correct UART/USART location.
    l_USART0->ROUTEPEN = USART_ROUTEPEN_RXPEN | USART_ROUTEPEN_TXPEN;
    l_USART0->ROUTELOC0 = (l_USART0->ROUTELOC0 &
                           ~(_USART_ROUTELOC0_TXLOC_MASK
                           | _USART_ROUTELOC0_RXLOC_MASK));

    // enable the UART
    USART_Enable(l_USART0, usartEnable);

    // Clear previous RX interrupts
    USART_IntClear(l_USART0, USART_IF_RXDATAV);
    NVIC_ClearPendingIRQ(USART0_RX_IRQn);

    // Enable RX interrupts
    USART_IntEnable(l_USART0, USART_IF_RXDATAV);

    // explicitly set NVIC priorities of all Cortex-M interrupts used
    NVIC_SetPriorityGrouping(0U);
    NVIC_SetPriority(USART0_RX_IRQn, 0U); // kernel unaware interrupt

    // enable the UART RX interrupt...
    NVIC_EnableIRQ(USART0_RX_IRQn); // UART0 interrupt used for QS-RX

    return 1U; // success
}
//............................................................................
void QS_onCleanup(void) {
    // wait as long as the UART is busy
    while ((l_USART0->STATUS & USART_STATUS_TXBL) == 0U) {
    }
    // delay before returning to allow all produced QS bytes to be received
    for (uint32_t volatile dly_ctr = 100000U; dly_ctr > 0U; --dly_ctr) {
    }
}
//............................................................................
// NOTE:
// No critical section in QS_onFlush() to avoid nesting of critical sections
// in case QS_onFlush() is called from Q_onError().
void QS_onFlush(void) {
    for (;;) {
        uint16_t b = QS_getByte();
        if (b != QS_EOD) {
            while ((l_USART0->STATUS & USART_STATUS_TXBL) == 0U) {
            }
            l_USART0->TXDATA = (uint8_t)b;
        }
        else {
            break;
        }
    }
}
//............................................................................
// callback function to reset the target (to be implemented in the BSP)
void QS_onReset(void) {
    NVIC_SystemReset();
}
//............................................................................
void QS_doOutput(void) {
    if ((l_USART0->STATUS & USART_STATUS_TXBL) != 0) {  // is TXE empty?
        QF_INT_DISABLE();
        uint16_t b = QS_getByte();
        QF_INT_ENABLE();

        if (b != QS_EOD) {  // not End-Of-Data?
            l_USART0->TXDATA = (uint8_t)b;
        }
    }
}
//............................................................................
void QS_onTestLoop() {
    QS_rxPriv_.inTestLoop = true;
    while (QS_rxPriv_.inTestLoop) {

        QS_rxParse();  // parse all the received bytes

        if ((l_USART0->STATUS & USART_STATUS_TXBL) != 0) { // is TXE empty?
            uint16_t b = QS_getByte();
            if (b != QS_EOD) {  // not End-Of-Data?
                l_USART0->TXDATA = (uint8_t)b;
            }
        }
    }
    // set inTestLoop to true in case calls to QS_onTestLoop() nest,
    // which can happen through the calls to QS_TEST_PAUSE().
    QS_rxPriv_.inTestLoop = true;
}
