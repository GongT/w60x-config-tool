#include "private.h"
#include <sys/socket.h>
#include <netdev.h>

static rt_bool_t old_state = RT_FALSE;

inline static const rt_bool_t is_open(rt_wlan_security_t t)
{
	return (t == SECURITY_WPS_OPEN) || (t == SECURITY_OPEN);
}

rt_err_t config_mode_disconnect_wifi()
{
	rt_wlan_scan_result_clean();
	netdev_dhcp_enabled(netdev_default, old_state);
	rt_wlan_disconnect();
	return 0;
}

rt_err_t config_mode_connect_wifi()
{
	rt_wlan_set_mode(RT_WLAN_DEVICE_STA_NAME, RT_WLAN_STATION);

	struct netdev *dev = CONFIG_INTERFACE_NETDEV;
	if (dev == NULL)
	{
#define STR_VALUE(arg) #arg
		KPINTF_COLOR(9, "can not find netdev: " STR_VALUE(CONFIG_INTERFACE_NETDEV) "\n");
		return RT_ERROR;
	}

	char config_wifi_ssid[128];
	memset(config_wifi_ssid, 0, sizeof(config_wifi_ssid));

	struct rt_wlan_scan_result *scan_result;
	KPINTF_LIGHT("Scan config wifi: " CONFIG_SERVER_SSID);
	for (; config_wifi_ssid[0] == '\0'; rt_thread_mdelay(3000))
	{
		KPINTF_DIM("scanning...");
		rt_wlan_scan_result_clean();

		scan_result = rt_wlan_scan_sync();
		if (!scan_result)
		{
			KPINTF_DIM("  * no result!");
			continue;
		}
		for (uint8_t index = 0; index < scan_result->num; index++)
		{
			struct rt_wlan_info *ele = &scan_result->info[index];

			if (CONFIG_WIFI_PASS == NULL && !is_open(ele->security))
			{
				KPINTF_DIM("  * %.*s - private: %x, skip", ele->ssid.len, ele->ssid.val, ele->security);
				continue;
			}
			else if (CONFIG_WIFI_PASS != NULL && is_open(ele->security))
			{
				KPINTF_DIM("  * %.*s - public, skip", ele->ssid.len, ele->ssid.val);
				continue;
			}

			if (str_prefix((char *)ele->ssid.val, sizeof(CONFIG_SERVER_SSID), CONFIG_SERVER_SSID) == 0)
			{
				KPINTF_DIM("  * %.*s - yes", ele->ssid.len, ele->ssid.val);
				strncpy(config_wifi_ssid, (char *)ele->ssid.val, ele->ssid.len);
				break;
			}
			KPINTF_DIM("  * %.*s - not related", ele->ssid.len, ele->ssid.val);
		}
	}
	rt_wlan_scan_result_clean();

	KPINTF_COLOR(14, "Connect WiFi: %s", config_wifi_ssid);
	if (rt_wlan_connect(config_wifi_ssid, CONFIG_WIFI_PASS) != RT_EOK)
	{
		KPINTF_COLOR(9, "wifi connect fail.");
		return RT_ERROR;
	}

	KPINTF_COLOR(10, "WiFi connected.");

	old_state = netdev_is_dhcp_enabled(dev);
#ifdef CONFIG_USE_STATIC
	ip_addr_t addr;
	assert(netdev_dhcp_enabled(dev, RT_FALSE) == 0);
	inet_aton("192.168.1.66", &addr);
	netdev_set_ipaddr(dev, &addr);
	inet_aton("192.168.1.1", &addr);
	netdev_set_gw(dev, &addr);
	inet_aton("255.255.255.0", &addr);
	netdev_set_netmask(dev, &addr);
#else
	assert(netdev_dhcp_enabled(dev, RT_TRUE) == 0);
#endif

	// ifconfig w0 192.168.1.30 192.168.1.1 255.255.255.0
	// uint wait = 0;
	while (!rt_wlan_is_ready())
	{
		rt_thread_mdelay(1000);
		// if (wait++ > 10)
		// {
		// 	KPINTF_COLOR(9, "Can not get IP address after 10 seconds.");
		// 	return RT_ETIMEOUT;
		// }
		if (!rt_wlan_is_connected())
		{
			KPINTF_COLOR(9, "WiFi connection broken before ready.");
			return RT_EIO;
		}
	}

	KPINTF_COLOR(10, "config WiFi is ready.");
	wifi_status_dump();
	return RT_EOK;
}
