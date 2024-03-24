//============================================================================
// Product: QUTEST port for MSP-EXP430F5529LP board
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

#include <msp430f5529.h>  // MSP430 variant used
// add other drivers if necessary...

//Q_DEFINE_THIS_MODULE("qutest_port")

using namespace QP;

// UART1 pins TX:P4.4, RX:P4.5
#define TXD     (1U << 4)
#define RXD     (1U << 5)

// Local-scope objects -------------------------------------------------------
// 1MHz clock setting, see BSP_init()
#define BSP_MCK     1000000U
#define BSP_SMCLK   1000000U

#define LED1        (1U << 0)
#define LED2        (1U << 7)

#define BTN_S1      (1U << 1)

//............................................................................
extern "C" {
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
    __interrupt void USCI_A1_ISR(void); // prototype
    #pragma vector=USCI_A1_VECTOR
    __interrupt void USCI_A1_ISR(void)
#elif defined(__GNUC__)
    __attribute__ ((interrupt(USCI_A1_VECTOR)))
    void USCI_A1_ISR(void)
#else
    #error MSP430 compiler not supported!
#endif
{
    if (UCA1IV == 2) {
        uint16_t b = UCA1RXBUF;
        QP::QS::rxPut(b);
    }
}

} // extern "C"

// QS callbacks ==============================================================
bool QS::onStartup(void const *arg) {
    static uint8_t qsBuf[256];  // buffer for QS; RAM is tight!
    static uint8_t qsRxBuf[80]; // buffer for QS receive channel
    //uint16_t tmp;

    WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer

    // leave the MCK and SMCLK at default DCO setting

    P1DIR |= LED1;  // set LED1 pin to output
    P4DIR |= LED2;  // set LED2 pin to output

    initBuf  (qsBuf,   sizeof(qsBuf));
    rxInitBuf(qsRxBuf, sizeof(qsRxBuf));

    // USCI setup code...
    P4SEL |= (RXD | TXD);  // select the UART function for the pins
    UCA1CTL1 |= UCSWRST;   // reset USCI state machine
    UCA1CTL1 |= UCSSEL_2;  // choose the SMCLK clock
#if 1
    UCA1BR0 = 6; // 1MHz 9600 (see User's Guide)
    UCA1BR1 = 0; // 1MHz 9600
    // modulation UCBRSx=0, UCBRFx=0, oversampling
    UCA1MCTL = UCBRS_0 | UCBRF_13 | UCOS16;
#else
    UCA1BR0 = 9; // 1MHz 115200 (see User's Guide)
    UCA1BR1 = 0; // 1MHz 115200
    UCA1MCTL |= UCBRS_1 | UCBRF_0; // modulation UCBRSx=1, UCBRFx=0
#endif
    UCA1CTL1 &= ~UCSWRST;  // initialize USCI state machine
    UCA1IE |= UCRXIE;      // Enable USCI_A1 RX interrupt

    return true; // return success
}
//............................................................................
void QS::onCleanup(void) {
    // wait as long as the UART is busy
    while ((UCA1STAT & UCBUSY) != 0U) { // TX busy?
    }
    // delay before returning to allow all produced QS bytes to be received
    for (std::uint32_t volatile dly_ctr = 1000U; dly_ctr > 0U; --dly_ctr) {
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
            while ((UCA1STAT & UCBUSY) != 0U) { // TX busy?
            }
            UCA1TXBUF = static_cast<std::uint8_t>(b);
        }
        else {
            break;
        }
    }
}
//............................................................................
// callback function to reset the target (to be implemented in the BSP)
void QS::onReset(void) {
    // write invalid password to WDT: cause a password-validation RESET
    WDTCTL = 0xDEAD;
}
//............................................................................
void QS::onTestLoop() {

    __enable_interrupt();  // IMPORTANT: enable global interrupts

    rxPriv_.inTestLoop = true;
    while (rxPriv_.inTestLoop) {

        // toggle the User LED on and then off, see NOTE01
        P4OUT |=  LED2;  // turn LED2 on
        P4OUT &= ~LED2;  // turn LED2 off

        rxParse();  // parse all the received bytes

        if ((UCA1STAT & UCBUSY) == 0U) { // TX NOT busy?

            QF_INT_DISABLE();
            std::uint16_t b = getByte();
            QF_INT_ENABLE();

            if (b != QS_EOD) {
                UCA1TXBUF = static_cast<std::uint8_t>(b);
            }
        }
    }
    // set inTestLoop to true in case calls to QS_onTestLoop() nest,
    // which can happen through the calls to QS_TEST_PAUSE().
    rxPriv_.inTestLoop = true;
}
