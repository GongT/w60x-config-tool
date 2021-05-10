#pragma once

#include "myflash.h"

extern struct fal_flash_dev w60x_onchip;

#ifdef MY_USING_SPI_FLASH
extern struct fal_flash_dev nor_flash0;
#define FAL_FLASH_DEV_TABLE \
	{                       \
		&w60x_onchip,       \
			&nor_flash0,    \
	}
#else
#define FAL_FLASH_DEV_TABLE \
	{                       \
		&w60x_onchip,       \
	}
#endif

#ifdef FAL_PART_HAS_TABLE_CFG

// ./packages/wm_libraries-latest/Include/Driver/wm_flash_map.h
#include "wm_flash_map.h"
#define INTERNAL_USER_START (0x80F0000UL - FLASH_BASE_ADDR)
#define INTERNAL_USER_END (0x80FBFFFUL - FLASH_BASE_ADDR)

#ifdef MY_USING_SPI_FLASH
#define FILESYSTEM {FAL_PART_MAGIC_WROD, FS_PARTITION_NAME, FAL_NOR_FLASH_NAME, 0, EXTERNAL_SIZE_MB * 1024 * 1024, 0},
#else
#define FILESYSTEM {FAL_PART_MAGIC_WROD, FS_PARTITION_NAME, "w60x_onchip", INTERNAL_USER_START, INTERNAL_USER_END - INTERNAL_USER_START, 0},
#endif

#define FAL_PART_TABLE \
	{                  \
		FILESYSTEM     \
	}

#endif /* _FAL_CFG_H_ */
