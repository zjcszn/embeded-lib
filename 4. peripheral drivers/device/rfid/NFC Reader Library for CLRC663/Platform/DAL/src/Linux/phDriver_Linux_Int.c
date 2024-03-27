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

#include "phDriver.h"

#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <poll.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#define PHDRIVER_LINUX_CFG_DIR                      "/sys/class/gpio"
#define PHDRIVER_LINUX_FILE_EXPORT_RETRY_COUNT      10
#define PHDRIVER_LINUX_FILE_EXPORT_TIMEOUT          100
#define PHDRIVER_LINUX_ERROR                        0xFFFFFFFF

#ifdef NXPBUILD__PHHAL_HW_PN5190
#define ENTER_CRITICAL _IO(101, 0)
#define EXIT_CRITICAL _IO(101, 1)
#define READ_IRQ _IO(101, 2)
#endif /* NXPBUILD__PHHAL_HW_PN5190 */


static void phDriver_Linux_IntDelay(uint32_t dwDelay);

phStatus_t PiGpio_is_exported(size_t gpio)
{
    uint32_t fd = 0;
    char bGpio[64] = {0};

    snprintf(bGpio, sizeof(bGpio), PHDRIVER_LINUX_CFG_DIR "/gpio%lu/direction", (long unsigned int) gpio);

    fd = open(bGpio, O_WRONLY);
    if (fd != PHDRIVER_LINUX_ERROR)
    {
        close(fd);
        return PH_DRIVER_SUCCESS;
    }

    return (PH_DRIVER_FAILURE | PH_COMP_DRIVER);
}


phStatus_t PiGpio_export(size_t gpio)
{
    uint32_t fd;
    uint32_t dwTimeoutLoop;
    char buf[64] = {0};
    size_t len = 0;

    if(PiGpio_is_exported(gpio) == PH_DRIVER_SUCCESS)
    {
        return PH_DRIVER_SUCCESS;
    }

    fd = open(PHDRIVER_LINUX_CFG_DIR "/export", O_WRONLY);
    if (fd == PHDRIVER_LINUX_ERROR)
    {
        return (PH_DRIVER_FAILURE | PH_COMP_DRIVER);
    }

    len = snprintf(buf, sizeof(buf), "%lu", (long unsigned int)gpio);
    write(fd, buf, len);
    close(fd);

    /* wait until file is actually available in user space OR TimeOut. */
    for (dwTimeoutLoop = 0; dwTimeoutLoop < PHDRIVER_LINUX_FILE_EXPORT_RETRY_COUNT; dwTimeoutLoop++)
    {
        if( PiGpio_is_exported(gpio) == 0 )
        {
            return PH_DRIVER_SUCCESS;
        }

        phDriver_Linux_IntDelay(PHDRIVER_LINUX_FILE_EXPORT_TIMEOUT);
    }

    return (PH_DRIVER_FAILURE | PH_COMP_DRIVER);
}

phStatus_t PiGpio_unexport(size_t gpio)
{
    uint32_t fd = 0;
    char buf[64] = {0};
    uint32_t dwLen = 0;

    fd = open(PHDRIVER_LINUX_CFG_DIR "/unexport", O_WRONLY);
    if (fd == PHDRIVER_LINUX_ERROR)
    {
        return (PH_DRIVER_FAILURE | PH_COMP_DRIVER);
    }

    dwLen = snprintf(buf, sizeof(buf), "%lu", (long unsigned int)gpio);
    write(fd, buf, dwLen);
    close(fd);

    return PH_DRIVER_SUCCESS;
}

phStatus_t PiGpio_set_direction(size_t gpio, bool output)
{
    uint32_t fd = 0;
    char buf[64] = {0};

    snprintf(buf, sizeof(buf), PHDRIVER_LINUX_CFG_DIR "/gpio%lu/direction", (long unsigned int)gpio);
    fd = open(buf, O_WRONLY);
    if (fd == PHDRIVER_LINUX_ERROR)
    {
        return (PH_DRIVER_FAILURE | PH_COMP_DRIVER);
    }

    if(output)
    {
        write(fd, "out", 3);
    }
    else
    {
        write(fd, "in", 2);
    }

    close(fd);

    return PH_DRIVER_SUCCESS;
}

phStatus_t PiGpio_set_edge(size_t gpio, bool rising, bool falling)
{
    uint32_t fd = 0;
    char buf[64] = {0};

    snprintf(buf, sizeof(buf), PHDRIVER_LINUX_CFG_DIR "/gpio%lu/edge", (long unsigned int)gpio);

    fd = open(buf, O_WRONLY);
    if (fd == PHDRIVER_LINUX_ERROR)
    {
        return (PH_DRIVER_FAILURE | PH_COMP_DRIVER);
    }

    if(rising && falling)
    {
        write(fd, "both", 4);
    }
    else if(rising)
    {
        write(fd, "rising", 6);
    }
    else
    {
        write(fd, "falling", 7);
    }

    close(fd);

    return PH_DRIVER_SUCCESS;
}

phStatus_t PiGpio_read(size_t gpio, uint8_t *pGpioVal)
{
    char path[30];
    char cValue;
    uint32_t fd;
    struct pollfd pollfd;

    *pGpioVal = 0;

    snprintf(path, 30, PHDRIVER_LINUX_CFG_DIR "/gpio%lu/value", (long unsigned int)gpio);
    fd = open(path, O_RDONLY);
    if (fd == PHDRIVER_LINUX_ERROR)
    {
        return (PH_DRIVER_FAILURE | PH_COMP_DRIVER);
    }

    pollfd.fd = fd;
    lseek(pollfd.fd, 0, SEEK_SET);

    if ((read(fd, &cValue, 1)) == PHDRIVER_LINUX_ERROR)
    {
        close(fd);
        return (PH_DRIVER_FAILURE | PH_COMP_DRIVER);
    }

    close(fd);

    *pGpioVal = (cValue == '0') ? 0 : 1;

    return PH_DRIVER_SUCCESS;
}

phStatus_t PiGpio_Write(size_t gpio, int value)
{
    const char s_values_str[] = "01";

    char path[30];
    uint32_t fd;

    snprintf(path, 30, PHDRIVER_LINUX_CFG_DIR "/gpio%lu/value", (long unsigned int)gpio);
    fd = open(path, O_WRONLY);
    if (fd == PHDRIVER_LINUX_ERROR)
    {
        return (PH_DRIVER_FAILURE | PH_COMP_DRIVER);
    }

    if (1 != write(fd, &s_values_str[(0 == value) ? 0 : 1], 1))
    {
        close(fd);
        return (PH_DRIVER_FAILURE | PH_COMP_DRIVER);
    }

    close(fd);
    return PH_DRIVER_SUCCESS;
}

phStatus_t PiGpio_poll(size_t gpio, int highOrLow, int timeOutms)
{
    char path[30];
    char cValue;
    uint32_t fd;
    uint32_t ret;
    struct pollfd pollfd;

    pollfd.events = POLLPRI; /* look for GPIO status change. */

    snprintf(path, 30, PHDRIVER_LINUX_CFG_DIR "/gpio%lu/value", (long unsigned int)gpio);
    fd = open(path, O_RDONLY);
    if (fd == PHDRIVER_LINUX_ERROR)
    {
        return (PH_DRIVER_FAILURE | PH_COMP_DRIVER);
    }

    pollfd.fd = fd; /* GPIO file descriptor. */

    ret = read(fd, &cValue, 1);
    if (ret == PHDRIVER_LINUX_ERROR)
    {
        close(fd);
        return (PH_DRIVER_FAILURE | PH_COMP_DRIVER);
    }

    if((highOrLow) == ((cValue == '0')? 0 : 1))
    {
        close(fd);
        return PH_DRIVER_SUCCESS;
    }

    /* Reposition the read/write file offset to 0 bytes. */
    lseek(pollfd.fd, 0, SEEK_SET);
    /* Poll until the GPIO state changes.
     * Num of Items in pollFd is 1. */
    ret = poll( &pollfd, 1, timeOutms );
    if( ret != 1 )
    {
        close(fd);
        return (PH_DRIVER_FAILURE | PH_COMP_DRIVER);
    }

    /* Check for GPIO change OR Timeout. */
    lseek(pollfd.fd, 0, SEEK_SET);

    ret = read(fd, &cValue, 1);
    if (ret == PHDRIVER_LINUX_ERROR)
    {
        close(fd);
        return (PH_DRIVER_FAILURE | PH_COMP_DRIVER);
    }

    if((highOrLow) == ((cValue == '0')? 0 : 1))
    {
        close(fd);
        return PH_DRIVER_SUCCESS;
    }else{
        close(fd);
        return (PH_DRIVER_TIMEOUT | PH_COMP_DRIVER);
    }
}


static void phDriver_Linux_IntDelay(uint32_t dwDelay)
{
    struct timespec sTimeSpecReq;
    struct timespec sTimeSpecRem;

    sTimeSpecReq.tv_sec = (dwDelay / 1000);
    sTimeSpecReq.tv_nsec = (dwDelay % 1000) * 1000 * 1000;

    nanosleep(&sTimeSpecReq, &sTimeSpecRem);
}

#ifdef NXPBUILD__PHHAL_HW_PN5190

typedef struct
{
    /* File descriptors */
    uint32_t irqFd;
}phIRQ_LinuxUserObj_t;

static phIRQ_LinuxUserObj_t sIRQObj;

phStatus_t PiGpio_OpenIrq()
{
    sIRQObj.irqFd          = PHDRIVER_LINUX_ERROR;

    sIRQObj.irqFd = open("/dev/irq", O_RDONLY);
    if(sIRQObj.irqFd == PHDRIVER_LINUX_ERROR)
    {
        return (PH_DRIVER_FAILURE | PH_COMP_DRIVER);
    }

    return PH_DRIVER_SUCCESS;
}

phStatus_t PiGpio_Irq()
{
    uint32_t dwStatus;
    int8_t  bValue;

    /* Set mode */
    dwStatus = read(sIRQObj.irqFd, &bValue, 1);
    if(dwStatus == PHDRIVER_LINUX_ERROR)
    {
        return (PH_DRIVER_FAILURE | PH_COMP_DRIVER);
    }

    return PH_DRIVER_SUCCESS;
}

phStatus_t PiGpio_IRQPin_Read()
{
    uint16_t dwStatus;

    dwStatus = ioctl(sIRQObj.irqFd, READ_IRQ, 0);

    if(dwStatus == PHDRIVER_LINUX_ERROR)
    {
        return (PH_DRIVER_FAILURE | PH_COMP_DRIVER);
    }

    return dwStatus;
}

phStatus_t PiGpio_Interrupt_Disable()
{
    uint32_t dwStatus;

    dwStatus = ioctl(sIRQObj.irqFd, ENTER_CRITICAL, 0);

    if(dwStatus == PHDRIVER_LINUX_ERROR)
    {
        return (PH_DRIVER_FAILURE | PH_COMP_DRIVER);
    }

    return PH_DRIVER_SUCCESS;
}

phStatus_t PiGpio_Interrupt_Enable()
{
    uint32_t dwStatus;

    dwStatus = ioctl(sIRQObj.irqFd, EXIT_CRITICAL, 0);

    if(dwStatus == PHDRIVER_LINUX_ERROR)
    {
        return (PH_DRIVER_FAILURE | PH_COMP_DRIVER);
    }

    return PH_DRIVER_SUCCESS;
}
#endif /* NXPBUILD__PHHAL_HW_PN5190 */

