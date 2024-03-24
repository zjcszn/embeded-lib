//============================================================================
// Product: QUTEST port for STM32 NUCLEO-H743ZI board
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
    #error "Q_SPY must be defined to compile qutest_cpp.cpp"
#endif // Q_SPY

#define QP_IMPL        // this is QP implementation
#include "qp_port.hpp" // QP port
#include "qs_port.hpp" // QS port
#include "qs_pkg.hpp"  // QS package-scope interface
#include "qsafe.h"     // QP Functional Safety (FuSa) Subsystem

// STM32CubeH7 include files
#include "stm32h7xx_hal.h"
#include "stm32h7xx_nucleo_144.h"
// add other drivers if necessary...

//Q_DEFINE_THIS_MODULE("qutest_cpp")

using namespace QP;

// ISRs defined in this BSP --------------------------------------------------
extern "C" void USART2_IRQHandler(void);

//............................................................................
extern "C" {

// ISR for receiving bytes from the QSPY Back-End
// NOTE: This ISR is "QF-unaware" meaning that it does not interact with
// the QF/QK and is not disabled. Such ISRs don't need to call QK_ISR_ENTRY/
// QK_ISR_EXIT and they cannot post or publish events.
//
void USART3_IRQHandler(void) { // kernel-unaware interrupt
    // is RX register NOT empty?
    if ((USART3->ISR & USART_ISR_RXNE_RXFNE) != 0U) {
        uint32_t b = USART3->RDR;
        QP::QS::rxPut(b);
        USART3->ISR &= ~USART_ISR_RXNE_RXFNE; // clear int.
    }
}

void assert_failed(char const * const module, int_t const id); // prototype
void assert_failed(char const * const module, int_t const id) {
    Q_onError(module, id);
}

} // extern "C"

// QS callbacks ==============================================================
bool QS::onStartup(void const *arg) {
    Q_UNUSED_PAR(arg);

    static std::uint8_t qsTxBuf[2*1024]; // buffer for QS-TX channel
    initBuf  (qsTxBuf, sizeof(qsTxBuf));

    static std::uint8_t qsRxBuf[256];    // buffer for QS-RX channel
    rxInitBuf(qsRxBuf, sizeof(qsRxBuf));

    static UART_HandleTypeDef uartHandle;
    uartHandle.Instance        = USART3;
    uartHandle.Init.BaudRate   = 115200;
    uartHandle.Init.WordLength = UART_WORDLENGTH_8B;
    uartHandle.Init.StopBits   = UART_STOPBITS_1;
    uartHandle.Init.Parity     = UART_PARITY_NONE;
    uartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
    uartHandle.Init.Mode       = UART_MODE_TX_RX;
    uartHandle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&uartHandle) != HAL_OK) {
        return 0U; // return failure
    }

    // Set UART to receive 1 byte at a time via interrupt
    HAL_UART_Receive_IT(&uartHandle, (uint8_t *)qsRxBuf, 1);

    NVIC_SetPriority(USART3_IRQn,  0U); // kernel unaware interrupt
    NVIC_EnableIRQ(USART3_IRQn); // UART interrupt used for QS-RX

    return true; // success
}
//............................................................................
void QS::onCleanup(void) {
    // wait as long as the UART is busy
    while ((USART3->ISR & UART_FLAG_TXE) == 0U) {
    }
    // delay before returning to allow all produced QS bytes to be received
    for (std::uint32_t volatile dly_ctr = 1000000U; dly_ctr > 0U; --dly_ctr) {
    }
}
//............................................................................
// NOTE:
// No critical section in QS::onFlush() to avoid nesting of critical sections
// in case QS_onFlush() is called from Q_onError().
void QS::onFlush(void) {
   for (;;) {
        std::uint16_t b = getByte();
        if (b != QS_EOD) { // not End-Of-Data?
            // busy-wait as long as UART-TX not ready
            while ((USART3->ISR & UART_FLAG_TXE) == 0U) {
            }
            USART3->TDR = static_cast<std::uint8_t>(b);
        }
        else {
            break;
        }
    }
}
//............................................................................
//! callback function to reset the target (to be implemented in the BSP)
void QS::onReset(void) {
    NVIC_SystemReset();
}
//............................................................................
void QS::doOutput(void) {
    if ((USART3->ISR & UART_FLAG_TXE) != 0U) { // TXE empty?

        QF_INT_DISABLE();
        std::uint16_t b = getByte();
        QF_INT_ENABLE();

        if (b != QS_EOD) {  // not End-Of-Data?
            USART3->TDR = static_cast<std::uint8_t>(b);
        }
    }
}
//............................................................................
void QS::onTestLoop() {
    rxPriv_.inTestLoop = true;
    while (rxPriv_.inTestLoop) {

        rxParse();  // parse all the received bytes

        if ((USART3->ISR & UART_FLAG_TXE) != 0U) {

            QF_INT_DISABLE();
            std::uint16_t b = getByte();
            QF_INT_ENABLE();

            if (b != QS_EOD) { // not End-Of-Data?
                USART3->TDR = static_cast<std::uint8_t>(b);
            }
        }
    }
    // set inTestLoop to true in case calls to QS_onTestLoop() nest,
    // which can happen through the calls to QS_TEST_PAUSE().
    rxPriv_.inTestLoop = true;
}
