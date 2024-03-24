/** @file moal_sdio_mmc.c
 *
 *  @brief This file contains SDIO MMC IF (interface) module
 *  related functions.
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
	02/25/09: Initial creation -
		  This file supports SDIO MMC only
****************************************************/

//#include <linux/firmware.h>

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <rtw_byteorder.h>

#include <hal_intf.h>
#include <moal_sdio.h>

#include <sdcard.h>
/********************************************************
		Local Variables
********************************************************/

/********************************************************
		Global Variables
********************************************************/

#ifdef SDIO_SUSPEND_RESUME
/** PM keep power */
extern int pm_keep_power;
#endif


/** WLAN IDs */
/*
static const struct sdio_device_id wlan_ids[] = {
    {SDIO_DEVICE(MARVELL_VENDOR_ID, SD_DEVICE_ID_8797)},
    {SDIO_DEVICE(MARVELL_VENDOR_ID, SD_DEVICE_ID_8782)},
    {},
};
*/
//MODULE_DEVICE_TABLE(sdio, wlan_ids);

int woal_sdio_probe(struct sdio_func *func);
void woal_sdio_remove(struct sdio_func *func);

#ifdef SDIO_SUSPEND_RESUME
#ifdef MMC_PM_KEEP_POWER
int woal_sdio_suspend(struct device *dev);
int woal_sdio_resume(struct device *dev);

static struct dev_pm_ops wlan_sdio_pm_ops = {
    .suspend = woal_sdio_suspend,
    .resume = woal_sdio_resume,
};
#endif
#endif

#if 0
static struct sdio_driver wlan_sdio = {
    .name = "wlan_sdio",
    .id_table = wlan_ids,
    .probe = woal_sdio_probe,
    .remove = woal_sdio_remove,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
    .drv = {
            .owner = THIS_MODULE,
#ifdef SDIO_SUSPEND_RESUME
#ifdef MMC_PM_KEEP_POWER
            .pm = &wlan_sdio_pm_ops,
#endif
#endif
            }
#else
#ifdef SDIO_SUSPEND_RESUME
#ifdef MMC_PM_KEEP_POWER
    .drv = {
            .pm = &wlan_sdio_pm_ops,
            }
#endif
#endif
#endif
};
#endif


u32 roundup(u32 x,u32 y) 
{							
	return (((x) + (y - 1)) / y) * y;
}
u32 rounddown(u32 x,u32 y) 
{							
	return x-(x%y);
}

static int sdio_io_rw_ext_helper(int write,
	unsigned addr, int incr_addr, u8 *buf, unsigned size)
{
	unsigned remainder = size;
	unsigned max_blocks;
	int ret;

	if (cccr.multi_block/* && (size > 256)*/){
		/* Blocks per command is limited by host count, host transfer
		 * size (we only use a single sg entry) and the maximum for
		 * IO_RW_EXTENDED of 511 blocks. */
		//p_dbg("sdio_io_rw_ext_helper,addr:%d,sz;%d,cur_blksize:%d \n",addr,size, func.cur_blksize);
		//¡Á¡é¨°a
//		if(func.cur_blksize == 256)
		max_blocks = 512;

		while (remainder > func.cur_blksize) {
			unsigned blocks;

			blocks = remainder / func.cur_blksize;
			if (blocks > max_blocks)
				blocks = max_blocks;
			size = blocks * func.cur_blksize;

			ret = mmc_io_rw_extended(write,
				func.num, addr, incr_addr, buf,
				blocks, func.cur_blksize);
			if (ret)
				return ret;

			remainder -= size;
			buf += size;
			if (incr_addr)
				addr += size;
		}
	}
	/* Write the remainder using byte mode. */
	while (remainder > 0) {
		size = min(remainder,256);

		ret = mmc_io_rw_extended(write, func.num, addr,
			 incr_addr, buf, 1, size);
		if (ret)
			return ret;

		remainder -= size;
		buf += size;
		if (incr_addr)
			addr += size;
	}
	return 0;
}


int sdio_reset(void)
{
	int ret;
	u8 abort;

	/* SDIO Simplified Specification V2.0, 4.4 Reset for SDIO */
	ret = mmc_io_rw_direct(0, 0, SDIO_CCCR_ABORT, 0, &abort);

	if (ret)
		abort = 0x08;
	else
		abort |= 0x08;

	ret = mmc_io_rw_direct(1, 0, SDIO_CCCR_ABORT, abort, NULL);

	return ret;
}


u8 sdio_readb(int fn,unsigned int addr, int *err_ret)
{
	int ret;
	u8 val = 0;

	if (err_ret)
		*err_ret = 0;
	
	ret = mmc_io_rw_direct(0, fn, addr, 0, &val);
	if (ret) {
		if (err_ret)
			*err_ret = ret;
//		return 0xFF;
	}

	return val;
}

u16 sdio_readw(int fn,unsigned int addr, int *err_ret)
{
	int ret;
	u16 val;

	ret = sdio_readsb(&val, addr, 2);
       if (ret) {
		if (err_ret)
			*err_ret = ret;
		return (u16)-1;
	}
    
	return val;
}

u32 sdio_readl(int fn,unsigned int addr, int *err_ret)
{
	int ret;
	u32 val;

	ret = sdio_readsb(&val, addr, 4);
    if (ret) {
		if (err_ret)
			*err_ret = ret;
		return (u32)-1;
	}
    
	return val;
}

void sdio_writew(int fn, uint16_t val, unsigned int addr, int *err_ret)
{
	sdio_writesb(addr, &val, 2);
}

void sdio_writel(int fn, uint32_t val,unsigned int addr, int *err_ret)
{
	sdio_writesb(addr, &val, 4);
}


void sdio_writeb(int fn,u8 b, unsigned int addr, int *err_ret)
{
	int ret;
	ret = mmc_io_rw_direct(1, fn, addr, b, NULL);
	if (err_ret)
	{
		*err_ret = ret;
	}
	if(ret){
		p_err("faild:%d",err_ret);
	}
}


u8 sdio_writeb_readb(int fn, u8 write_byte,
	unsigned int addr, int *err_ret)
{
	int ret;
	u8 val;

	ret = mmc_io_rw_direct(1, fn, addr,
			write_byte, &val);
	if (err_ret)
		*err_ret = ret;
	if (ret)
		val = 0xff;

	return val;
}


int sdio_readsb(void *dst, unsigned int addr,
	int count)
{
	return sdio_io_rw_ext_helper(0, addr, 0, dst, count);
}


int sdio_writesb(unsigned int addr, void *src,
	int count)
{
	return sdio_io_rw_ext_helper(1, addr, 0, src, count);
}

unsigned char sdio_f0_readb(unsigned int addr, int *err_ret)
{
	int ret;
	unsigned char val;

	if (err_ret)
		*err_ret = 0;

	ret = mmc_io_rw_direct(0, 0, addr, 0, &val);
	if (ret) {
		if (err_ret)
			*err_ret = ret;
		return 0xFF;
	}

	return val;
}

void sdio_f0_writeb(unsigned char b, unsigned int addr,
	int *err_ret)
{
	int ret;

	if ((addr < 0xF0 || addr > 0xFF) /*&& (!mmc_card_lenient_fn0(func->card))*/) {
		if (err_ret)
			*err_ret = -EINVAL;
		return;
	}

	ret = mmc_io_rw_direct(1, 0, addr, b, NULL);
	if (err_ret)
		*err_ret = ret;
}

int sdio_set_block_size(int fn,unsigned blksz)
{
	int ret = 0;
	ret = mmc_io_rw_direct(1, 0,
		SDIO_FBR_BASE(fn) + SDIO_FBR_BLKSIZE,
		blksz & 0xff, NULL);
	if (ret)
		goto end;
	ret = mmc_io_rw_direct(1, 0,
		SDIO_FBR_BASE(fn) + SDIO_FBR_BLKSIZE + 1,
		(blksz >> 8) & 0xff, NULL);

	func.cur_blksize = blksz;
	func.max_blksize = 512;
	if (ret)
		goto end;
end:
	return ret;
}


int sdio_enable_func(struct sdio_func *func)
{
	int ret;
	unsigned char reg;
	
	ret = mmc_io_rw_direct(0, 0, SDIO_CCCR_IOEx, 0, &reg);
	if (ret  != SD_OK)
		goto err;

	 func->num = 1;

	reg |= 1 << func->num;

	ret = mmc_io_rw_direct(1, 0, SDIO_CCCR_IOEx, reg, NULL);
	if (ret  != SD_OK)
		goto err;

	while (1) {
		ret = mmc_io_rw_direct(0, 0, SDIO_CCCR_IORx, 0, &reg);
		if (ret  != SD_OK)
			goto err;
		if (reg & (1 << func->num))
			break;
	}

	p_dbg("SDIO: Enabled func %d\n", reg);

	return 0;

err:
	p_err("SDIO: Failed to enable device\n");
	return ret;
}


int sdio_disable_func(struct sdio_func *func)
{
	int ret;
	unsigned char reg;


	ret = mmc_io_rw_direct(0, 0, SDIO_CCCR_IOEx, 0, &reg);
	if (ret)
		goto err;

	reg &= ~(1 << func->num);

	ret = mmc_io_rw_direct(1, 0, SDIO_CCCR_IOEx, reg, NULL);
	if (ret)
		goto err;

	return 0;

err:
	p_err("SDIO: Failed to disable device\n");
	return -EIO;
}


int sdio_claim_irq(struct sdio_func *func, void *handler)
{
	int ret;
	unsigned char reg;

	ret = mmc_io_rw_direct(0, 0, SDIO_CCCR_IENx, 0, &reg);
	if (ret)
	{
		p_err("sdio_claim_irq err\n");
		return ret;
	}
	p_dbg("sdio_claim_irq1 :%x\n", reg);	
	
	reg |= 1 << func->num;
	reg |= 0x01; /* Master interrupt enable */

	ret = mmc_io_rw_direct(1, 0, SDIO_CCCR_IENx, reg, NULL);
	if (ret)
		p_err("sdio_claim_irq err1\n");

	ret = mmc_io_rw_direct(0, 0, SDIO_CCCR_IENx, 0, &reg);
	p_dbg("sdio_claim_irq2 :%x\n", reg);	

	func->sdio_int_wait = init_event();
	thread_create((void (*)(void *))handler, func, TASK_SDIO_THREAD_PRIO, 0, TASK_SDIO_STACK_SIZE, "sdio_thread");
	
	enable_sdio_int();
	
	return ret;
}

int sdio_release_irq(struct sdio_func *func)
{
	int ret;
	unsigned char reg;


	ret = mmc_io_rw_direct(0, 0, SDIO_CCCR_IENx, 0, &reg);
	if (ret)
		return ret;

	reg &= ~(1 << func->num);

	/* Disable master interrupt with the last function interrupt */
	if (!(reg & 0xFE))
		reg = 0;

	ret = mmc_io_rw_direct(1, 0, SDIO_CCCR_IENx, reg, NULL);
	if (ret)
		return ret;

	return 0;
}


