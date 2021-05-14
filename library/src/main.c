#define DBG_TAG "CFG:MAIN"

#include "private.h"
#include <rthw.h>

__attribute__((noreturn)) inline static void reboot()
{

	for (int i = 5; i > 0; i--)
	{
		LOG_E("\rreboot system in %d", i);
		rt_thread_mdelay(1000);
	}
	LOG_E("\rreboot...");
	rt_hw_cpu_reset();
	while (1)
		;
}

static int goto_config_mode_inner(rt_bool_t done_reboot)
{
	LOG_I("Entering config mode...");
	config_mode_status_hook(CONFIG_STATUS_INIT);

	if (!rt_wlan_is_ready())
	{
		if (config_mode_connect_wifi() != RT_EOK)
		{
			config_mode_status_hook(CONFIG_STATUS_WIFI_FAIL);
			if (done_reboot)
				reboot();
			return 1;
		}
	}

	if (config_mode_connect_tcp() != RT_EOK)
	{
		config_mode_status_hook(CONFIG_STATUS_TCP_FAIL);
		if (done_reboot)
			reboot();
		return 1;
	}

	do
	{
		FOREACH_CONFIG(item)
		{
			LOG_I(" * %s", item);
		}
	} while (0);

	FOREACH_CONFIG(item)
	{
		LOG_I("get config: %s", item);
		const char *value = config_request_data(item);

		if (value == NULL)
		{
			config_mode_status_hook(CONFIG_STATUS_ITEM_MISSING);
			LOG_E("missing config: %s", item);
			if (done_reboot)
				reboot();
		}

		LOG_I("got config [%s] is [%s]", item, value);
		if (save_config_item(item, value) != RT_EOK)
		{
			config_mode_status_hook(CONFIG_STATUS_STORAGE_FAIL);
			LOG_E("failed save config %s!", item);
			if (done_reboot)
				reboot();
		}
	}

	LOG_I("Done config mode!");
	if (done_reboot)
		reboot();
	return 0;
}

__attribute__((weak)) void config_mode_status_hook(enum CONFIG_STATUS pos)
{
}

__attribute__((noreturn)) void goto_config_mode()
{
	goto_config_mode_inner(RT_TRUE);
	while (1)
		;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
static int goto_config_mode_shell(void)
{
	if (rt_wlan_is_connected())
	{
		LOG_I("WiFi disconnect...");
		if (rt_wlan_disconnect() != RT_EOK)
		{
			LOG_E("failed disconnect WiFi.");
			return 1;
		}
	}
	return goto_config_mode_inner(RT_FALSE);
}

MSH_CMD_EXPORT_ALIAS(goto_config_mode_shell, exec_wifi_config, start WiFi config mode);
#endif
