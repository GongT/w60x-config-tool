#define DBG_TAG "STOR"

#include "app.h"
#include "myflash.h"
#include <fal.h>
#include <easyflash.h>
#include <easyflash-extend.h>

extern int fal_init_check();

extern const struct dfs_filesystem_ops *filesystem_operation_table[DFS_FILESYSTEM_TYPES_MAX];

#define BUF_SIZE 1024

#ifdef RT_USING_FINSH
#include <finsh.h>
static void test_block()
{
	int ret;
	int j, len;
	size_t i;
	uint8_t buf[BUF_SIZE];
	const struct fal_flash_dev *flash_dev = RT_NULL;
	const struct fal_partition *partition = RT_NULL;

	if (!FS_PARTITION_NAME)
	{
		FATAL_ERROR("Input param partition name is null!");
	}

	partition = fal_partition_find(FS_PARTITION_NAME);
	if (partition == RT_NULL)
	{
		FATAL_ERROR("Find partition (%s) failed!", FS_PARTITION_NAME);
	}

	flash_dev = fal_flash_device_find(partition->flash_name);
	if (flash_dev == RT_NULL)
	{
		FATAL_ERROR("Find flash device (%s) failed!", partition->flash_name);
	}

	printf("==================================\n"
		   "Flash device: %s\n"
		   "  size: %dK\n"
		   "  partition: %s\n"
		   "  partition size: %dK\n\n",
		   partition->flash_name,
		   flash_dev->len / 1024,
		   partition->name,
		   partition->len / 1024);

	/* 擦除 `partition` 分区上的全部数据 */
	int esize = fal_partition_erase_all(partition);
	if (esize == 0)
	{
		FATAL_ERROR("Partition (%s) erase failed!", partition->name);
	}
	LOG_W("Partition (%s) data erased (%d)!", FS_PARTITION_NAME, esize);

	/* 循环读取整个分区的数据，并对内容进行检验 */
	for (i = 0; i < partition->len;)
	{
		rt_memset(buf, 0x00, BUF_SIZE);
		len = (partition->len - i) > BUF_SIZE ? BUF_SIZE : (partition->len - i);

		/* 从 Flash 读取 len 长度的数据到 buf 缓冲区 */
		ret = fal_partition_read(partition, i, buf, len);
		if (ret < 0)
		{
			FATAL_ERROR("Partition (%s) read failed!", partition->name);
		}
		for (j = 0; j < len; j++)
		{
			/* 校验数据内容是否为 0xFF */
			if (buf[j] != 0xFF)
			{
				printf("\n\nread block len=%d\n", len);
				printf("read fail: 0x%x + 0x%x = 0x%x\n", i, j, buf[j]);
				FATAL_ERROR("The erase operation did not really succeed!");
			}
		}
		i += len;
		printf("read 0xFFs: %d/%d\r", i, partition->len);
	}
	LOG_I("Read (%s) partition finish! Write size %d(%dK).", FS_PARTITION_NAME, i, i / 1024);

	/* 把 0 写入指定分区 */
	rt_memset(buf, 0x00, BUF_SIZE);
	for (i = 0; i < partition->len;)
	{
		/* 设置写入的数据 0x00 */
		len = (partition->len - i) > BUF_SIZE ? BUF_SIZE : (partition->len - i);

		/* 写入数据 */
		ret = fal_partition_write(partition, i, buf, len);
		if (ret < 0)
		{
			printf("\n\nread block len=%d\n", len);
			printf("write fail: 0x%x + 0x%x = 0x%x\n", i, j, buf[j]);
			FATAL_ERROR("Partition (%s) write failed!", partition->name);
		}
		i += len;
		printf("write zeros: %d/%d\r", i, partition->len);
	}
	printf("\r");
	LOG_I("Write (%s) partition finish! Write size %d(%dK).", FS_PARTITION_NAME, i, i / 1024);

	/* 从指定的分区读取数据并校验数据 */
	for (i = 0; i < partition->len;)
	{
		/* 清空读缓冲区，以 0xFF 填充 */
		rt_memset(buf, 0xFF, BUF_SIZE);
		len = (partition->len - i) > BUF_SIZE ? BUF_SIZE : (partition->len - i);

		/* 读取数据到 buf 缓冲区 */
		ret = fal_partition_read(partition, i, buf, len);
		if (ret < 0)
		{
			FATAL_ERROR("Partition (%s) read failed!", partition->name);
		}
		for (j = 0; j < len; j++)
		{
			/* 校验读取的数据是否为步骤 3 中写入的数据 0x00 */
			if (buf[j] != 0x00)
			{
				printf("\n\nread block len=%d\n", len);
				printf("read fail: 0x%x + 0x%x = 0x%x\n", i, j, buf[j]);
				FATAL_ERROR("The write operation did not really succeed!");
			}
		}
		i += len;
		printf("read zeros: %d/%d\r", i, partition->len);
	}
	LOG_I("Read (%s) partition finish! Write size %d(%dK).", FS_PARTITION_NAME, i, i / 1024);
}

MSH_CMD_EXPORT(test_block, test filesystem block device);
#endif

static int init_storage()
{
	LOG_I("-- init_storage() --");
	if (fal_init_check() == 0)
	{
		if (fal_init() <= 0)
		{
			FATAL_ERROR("No partitions found!");
		}
	}

#if !FAL_DEBUG
	fal_show_part_table();
#endif

	if (easyflash_init() != EF_NO_ERR)
	{
		FATAL_ERROR("Failed to init easyflash!");
	}

	test_env();

	return 0;
}
INIT_COMPONENT_EXPORT(init_storage);
