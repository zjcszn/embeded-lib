/*----------------------------------------------------------------------------*/
/* Copyright 2017-2022 NXP                                                    */
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

#ifndef PHPLATFORM_PORT_LINUXGPIO_H_
#define PHPLATFORM_PORT_LINUXGPIO_H_

#include <phDriver.h>


#include <stddef.h>
#include <stdbool.h>

phStatus_t PiGpio_is_exported(size_t gpio);
phStatus_t PiGpio_export(size_t gpio);
phStatus_t PiGpio_unexport(size_t gpio);
phStatus_t PiGpio_set_direction(size_t gpio, bool output);
phStatus_t PiGpio_set_edge(size_t gpio, bool rising, bool falling);
phStatus_t PiGpio_Write(size_t gpio, int value);
phStatus_t PiGpio_read(size_t gpio, uint8_t *pGpioVal);
phStatus_t PiGpio_poll(size_t gpio, int highOrLow, int timeOutms);

#ifdef NXPBUILD__PHHAL_HW_PN5190
phStatus_t PiGpio_OpenIrq();
phStatus_t PiGpio_Irq();
phStatus_t PiGpio_Interrupt_Disable();
phStatus_t PiGpio_Interrupt_Enable();
phStatus_t PiGpio_IRQPin_Read();
#endif /* NXPBUILD__PHHAL_HW_PN5190 */

#endif /* PHPLATFORM_PORT_LINUXGPIO_H_ */
