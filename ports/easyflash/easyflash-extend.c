#include <rtthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "easyflash-extend.h"

void test_env(void)
{
	uint32_t i_boot_times = 0;
	char c_boot_times[12] = {0, 0, 0, 0, 0};

	if (ef_get_env_blob("boot_times", c_boot_times, sizeof c_boot_times, NULL) == 0)
	{
		i_boot_times = 0;
	}
	else
	{
		i_boot_times = atol(c_boot_times);
	}
	i_boot_times++;

	sprintf(c_boot_times, "%ld", i_boot_times);
	ef_set_env("boot_times", c_boot_times);

	ef_print_env();
}

size_t get_storage_size(const char *key)
{
	size_t r;
	if (ef_get_env_blob(key, NULL, SIZE_MAX, &r) == 0)
	{
		return 0;
	}
	return r;
}
