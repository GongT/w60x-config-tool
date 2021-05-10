#pragma once

#define FAL_NOR_FLASH_NAME "norflash"
#define FS_PARTITION_NAME "fs"
#define FAL_EF_PART_NAME FS_PARTITION_NAME

#ifdef RT_USING_SFUD
#define EXTERNAL_SIZE_MB 0

#if EXTERNAL_SIZE_MB > 0
#define MY_USING_SPI_FLASH
#endif
#endif
