#define DBG_TAG "CFG:WIFI"

#include "private.h"
#include <sys/socket.h>
#include <netdev.h>

rt_err_t config_mode_disconnect_wifi()
{
	rt_wlan_scan_result_clean();
	struct netdev *dev = netdev_get_by_name("w0");
	if (dev != NULL)
		netdev_dhcp_enabled(dev, 0);

	rt_wlan_disconnect();
}

rt_err_t config_mode_connect_wifi()
{
	rt_wlan_set_mode(RT_WLAN_DEVICE_STA_NAME, RT_WLAN_STATION);
	char config_wifi_ssid[128];
	memset(config_wifi_ssid, 0, sizeof(config_wifi_ssid));

	struct rt_wlan_scan_result *scan_result;
	LOG_I("Scan config wifi: " CONFIG_SERVER_SSID);
	for (; config_wifi_ssid[0] == '\0'; rt_thread_mdelay(3000))
	{
		LOG_D("scanning...");
		rt_wlan_scan_result_clean();

		scan_result = rt_wlan_scan_sync();
		if (!scan_result)
		{
			LOG_D("  * no result!");
			continue;
		}
		for (uint8_t index = 0; index < scan_result->num; index++)
		{
			struct rt_wlan_info *ele = &scan_result->info[index];

			if (CONFIG_WIFI_PASS == NULL && ele->security != SECURITY_OPEN)
			{
				LOG_D("  * %.*s - private, skip", ele->ssid.len, ele->ssid.val);
				continue;
			}
			else if (CONFIG_WIFI_PASS != NULL && ele->security == SECURITY_OPEN)
			{
				LOG_D("  * %.*s - public, skip", ele->ssid.len, ele->ssid.val);
				continue;
			}

			if (str_prefix((char *)ele->ssid.val, sizeof(CONFIG_SERVER_SSID), CONFIG_SERVER_SSID) == 0)
			{
				LOG_D("  * %.*s - yes", ele->ssid.len, ele->ssid.val);
				strncpy(config_wifi_ssid, (char *)ele->ssid.val, ele->ssid.len);
				break;
			}
			LOG_D("  * %.*s - not related", ele->ssid.len, ele->ssid.val);
		}
	}
	rt_wlan_scan_result_clean();

	LOG_I("Connect WiFi: %s", config_wifi_ssid);
	if (rt_wlan_connect(config_wifi_ssid, CONFIG_WIFI_PASS) != RT_EOK)
	{
		LOG_E("wifi connect fail.");
		return RT_ERROR;
	}

	LOG_I("WiFi connected.");

	struct netdev *dev = netdev_get_by_name("w0");
	while (dev == NULL)
	{
		dev = netdev_get_by_name(RT_WLAN_DEVICE_STA_NAME);
		LOG_I("can not find netdev name: " RT_WLAN_DEVICE_STA_NAME "\n");
		rt_thread_mdelay(5000);
	}
	assert(netdev_dhcp_enabled(dev, 1) == 0);

	uint wait = 0;
	while (!rt_wlan_is_ready())
	{
		rt_thread_mdelay(1000);
		if (wait++ > 10)
		{
			LOG_E("Can not get IP address after 10 seconds.");
			return RT_ETIMEOUT;
		}
		if (!rt_wlan_is_connected())
		{
			LOG_E("WiFi connection broken before ready.");
			return RT_EIO;
		}
	}

	LOG_I("config WiFi is ready.");
	wifi_status_dump();
	return RT_EOK;
}
