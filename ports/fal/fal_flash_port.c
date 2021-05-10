/*
 * File      : fal_flash_sfud_port.c
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-01-26     armink       the first version
 * 2018-08-21     MurphyZhao   update to stm32l4xx
 */

#include <fal.h>
#include <drv_flash.h>
#include "myflash.h"

static int init(void)
{
	wm_flash_init();
	w60x_onchip.len = wm_flash_total();
	w60x_onchip.blk_size = wm_flash_blksize();
	w60x_onchip.addr = wm_flash_addr();

	return 0;
}

static int read(long offset, uint8_t *buf, size_t size)
{
	return wm_flash_read(offset, buf, size);
}

static int write(long offset, const uint8_t *buf, size_t size)
{
	return wm_flash_write(offset, (void *)buf, size);
}

static int erase(long offset, size_t size)
{
	return wm_flash_erase(offset, size);
}

struct fal_flash_dev w60x_onchip = {"w60x_onchip", 0, 0, 0, {init, read, write, erase}};
