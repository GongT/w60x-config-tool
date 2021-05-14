#define DBG_TAG "CFG:MAIN"

#include "private.h"
#include <rthw.h>

static void cleanup()
{
	config_mode_disconnect_tcp();
	config_mode_disconnect_wifi();
}

enum CONFIG_STATUS goto_config_mode()
{
	LOG_I("Entering config mode...");

	if (!rt_wlan_is_ready())
	{
		if (config_mode_connect_wifi() != RT_EOK)
		{
			cleanup();
			return CONFIG_STATUS_WIFI_FAIL;
		}
	}

	if (config_mode_connect_tcp() != RT_EOK)
	{
		rt_wlan_disconnect();
		cleanup();
		return CONFIG_STATUS_TCP_FAIL;
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
			LOG_E("missing config: %s", item);
			cleanup();
			return CONFIG_STATUS_ITEM_MISSING;
		}

		LOG_I("got config [%s] is [%s]", item, value);
		if (save_config_item(item, value) != RT_EOK)
		{
			LOG_E("failed save config %s!", item);
			cleanup();
			return CONFIG_STATUS_STORAGE_FAIL;
		}
	}

	LOG_I("Done config mode!");
	cleanup();
	return CONFIG_STATUS_SUCCESS;
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
	return goto_config_mode();
}

MSH_CMD_EXPORT_ALIAS(goto_config_mode_shell, exec_wifi_config, start WiFi config mode);
#endif
