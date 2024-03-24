/** @file moal_sdio.h
  *
  * @brief This file contains definitions for SDIO interface.
  * driver. 
  *
  * Copyright (C) 2008-2011, Marvell International Ltd. 
  * 
  * This software file (the "File") is distributed by Marvell International 
  * Ltd. under the terms of the GNU General Public License Version 2, June 1991 
  * (the "License").  You may use, redistribute and/or modify this File in 
  * accordance with the terms and conditions of the License, a copy of which 
  * is available by writing to the Free Software Foundation, Inc.,
  * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA or on the
  * worldwide web at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
  *
  * THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE 
  * IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE 
  * ARE EXPRESSLY DISCLAIMED.  The License provides additional details about 
  * this warranty disclaimer.
  *
  */
/****************************************************
Change log:
****************************************************/

#ifndef	_MOAL_SDIO_H
#define	_MOAL_SDIO_H
/*
#include        <linux/mmc/sdio.h>
#include        <linux/mmc/sdio_ids.h>
#include        <linux/mmc/sdio_func.h>
#include        <linux/mmc/card.h>
#include        <linux/mmc/host.h>
*/



#ifndef BLOCK_MODE
/** Block mode */
#define BLOCK_MODE	1
#endif

#ifndef BYTE_MODE
/** Byte Mode */
#define BYTE_MODE	0
#endif

#ifndef FIXED_ADDRESS
/** Fixed address mode */
#define FIXED_ADDRESS	0
#endif

#ifdef STA_SUPPORT
/** Default firmware name */

#define DEFAULT_FW_NAME	"mrvl/sd8797_uapsta.bin"
#define DEFAULT_FW_NAME_8782 "mrvl/sd8782_uapsta.bin"

#ifndef DEFAULT_FW_NAME
#define DEFAULT_FW_NAME ""
#endif
#endif /* STA_SUPPORT */

#ifdef UAP_SUPPORT
/** Default firmware name */

#define DEFAULT_AP_FW_NAME "mrvl/sd8797_uapsta.bin"
#define DEFAULT_AP_FW_NAME_8782 "mrvl/sd8782_uapsta.bin"

#ifndef DEFAULT_AP_FW_NAME
#define DEFAULT_AP_FW_NAME ""
#endif
#endif /* UAP_SUPPORT */

/** Default firmaware name */

#define DEFAULT_AP_STA_FW_NAME "mrvl/sd8797_uapsta.bin"
#define DEFAULT_AP_STA_FW_NAME_8782 "mrvl/sd8782_uapsta.bin"

#ifndef DEFAULT_AP_STA_FW_NAME
#define DEFAULT_AP_STA_FW_NAME ""
#endif

/*
 * SDIO function devices
 */
struct sdio_func {

	unsigned int		num;		/* function number */

	unsigned char		class;		/* standard interface class */
	unsigned short		vendor;		/* vendor id */
	unsigned short		device;		/* device id */

	unsigned		max_blksize;	/* maximum block size */
	unsigned		cur_blksize;	/* current block size */

	unsigned		enable_timeout;	/* max enable timeout in msec */

	unsigned int		state;		/* function state */
#define SDIO_STATE_PRESENT	(1<<0)		/* present in sysfs */

	unsigned char			tmpbuf[4];	/* DMA:able scratch buffer */

	unsigned		num_info;	/* number of info strings */
	const char		**info;		/* info strings */

	void *dev_data;
	wait_event_t sdio_int_wait;

};

unsigned char sdio_f0_readb(unsigned int addr, int *err_ret);
void sdio_f0_writeb(unsigned char b, unsigned int addr,
	int *err_ret);
int sdio_readsb(void *dst, unsigned int addr,
	int count);


int sdio_writesb(unsigned int addr, void *src,
	int count);
int sdio_set_block_size(int fn,unsigned blksz);
unsigned char sdio_readb(int fn,unsigned int addr, int *err_ret);
void sdio_writeb(int fn,unsigned char  b, unsigned int addr, int *err_ret);
int woal_sdio_probe(struct sdio_func *func);
int sdio_reset(void);
void sdio_writew(int fn, uint16_t val, unsigned int addr, int *err_ret);
void sdio_writel(int fn, uint32_t val,unsigned int addr, int *err_ret);
int sdio_release_irq(struct sdio_func *func);
int sdio_claim_irq(struct sdio_func *func, void *handler);
u16 sdio_readw(int fn,unsigned int addr, int *err_ret);
u32 sdio_readl(int fn,unsigned int addr, int *err_ret);
int sdio_enable_func(struct sdio_func *func);
int sdio_disable_func(struct sdio_func *func);

#endif /* _MOAL_SDIO_H */
