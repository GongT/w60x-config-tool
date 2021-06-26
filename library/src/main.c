#include "private.h"
#include <rthw.h>

static void cleanup()
{
	config_mode_disconnect_tcp();
	config_mode_disconnect_wifi();
}

static enum CONFIG_STATUS init()
{
	if (!rt_wlan_is_ready())
	{
		if (config_mode_connect_wifi() != RT_EOK)
		{
			return CONFIG_STATUS_WIFI_FAIL;
		}
	}

	if (config_mode_connect_tcp() != RT_EOK)
	{
		return CONFIG_STATUS_TCP_FAIL;
	}
	return CONFIG_STATUS_SUCCESS;
}

#define WRAP_CONNECT(DEBUG_MSG, FN)   \
	KPRINTF_COLOR(9, DEBUG_MSG);      \
	enum CONFIG_STATUS ret = init();  \
	if (ret == CONFIG_STATUS_SUCCESS) \
		ret = FN();                   \
	cleanup();                        \
	return ret

inline static enum CONFIG_STATUS goto_config_mode_inline()
{
	int ota_result = config_mode_OTA();
	if (ota_result != CONFIG_STATUS_SUCCESS && ota_result != CONFIG_STATUS_REBOOT_REQUIRED)
		return ota_result;

	enum CONFIG_STATUS success_return = ota_result;

	config_mode_status_callback(CONFIG_STATUS_CONFIG_START);

	do
	{
		FOREACH_CONFIG(item)
		{
			KPRINTF_DIM(" * %s", item);
		}
	} while (0);

	uint8_t mac[6];
	char mac_str[13];
	if (rt_wlan_get_mac(mac) != RT_EOK)
	{
		return CONFIG_STATUS_NO_MAC;
	}
	rt_sprintf(mac_str, "%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	FOREACH_CONFIG(item)
	{
		config_mode_status_callback(CONFIG_STATUS_HTTP_SEND);
		KPRINTF_LIGHT("get config: %s", item);
		const char *value = config_mode_request_data(item, mac_str);

		if (value == NULL)
		{
			KPRINTF_COLOR(11, "transform failed when get config: %s", item);
			return CONFIG_STATUS_HTTP_FAIL;
		}
		if (strlen(value) == 0)
		{
			KPRINTF_COLOR(11, "missing config: %s", item);
			return CONFIG_STATUS_ITEM_MISSING;
		}

		KPRINTF_LIGHT("got config [%s] is [%s]", item, value);

		config_mode_status_callback(CONFIG_STATUS_FLASH_WRITE);
		if (save_config_item(item, value) != RT_EOK)
		{
			KPRINTF_COLOR(11, "failed save config %s!", item);
			return CONFIG_STATUS_STORAGE_FAIL;
		}
	}

	KPRINTF_COLOR(10, "Done config mode!");

	config_mode_status_callback(CONFIG_STATUS_CONFIG_END);
	return success_return;
}

enum CONFIG_STATUS goto_config_mode()
{
	config_mode_status_callback(CONFIG_STATUS_WIFI_CONNECT);
	WRAP_CONNECT("Entering config mode...", goto_config_mode_inline);
}

enum CONFIG_STATUS goto_config_mode_OTA()
{
	config_mode_status_callback(CONFIG_STATUS_WIFI_CONNECT);
	WRAP_CONNECT("Entering OTA mode...", config_mode_OTA);
}

#ifdef RT_USING_FINSH
#include <finsh.h>
inline static rt_bool_t disconnect()
{
	if (rt_wlan_is_connected())
	{
		KPRINTF_COLOR(9, "WiFi disconnect...");
		if (rt_wlan_disconnect() != RT_EOK)
		{
			KPRINTF_COLOR(9, "failed disconnect WiFi.");
			return RT_FALSE;
		}
	}
	return RT_TRUE;
}
static int goto_config_mode_shell(void)
{
	if (!disconnect())
		return 1;
	return goto_config_mode();
}
MSH_CMD_EXPORT_ALIAS(goto_config_mode_shell, exec_wifi_config, start WiFi config mode);

static int goto_config_mode_OTA_shell(void)
{
	if (!disconnect())
		return 1;
	return goto_config_mode_OTA();
}
MSH_CMD_EXPORT_ALIAS(goto_config_mode_OTA_shell, exec_wifi_ota, start WiFi OTA mode);
#endif

RT_WEAK void config_mode_status_callback(enum CONFIG_STATUS status)
{
}
