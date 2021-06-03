#include "private.h"
#include <sys/socket.h>
#include <netdev.h>

static rt_bool_t old_state = RT_FALSE;
static char *config_wifi_ssid = NULL;
#define MAX_SSID_LENGTH 128

inline static const rt_bool_t is_open(rt_wlan_security_t t)
{
	return (t == SECURITY_WPS_OPEN) || (t == SECURITY_OPEN);
}

rt_err_t config_mode_disconnect_wifi()
{
	rt_wlan_scan_result_clean();
	netdev_dhcp_enabled(netdev_default, old_state);
	rt_wlan_disconnect();

	if (config_wifi_ssid != NULL)
		free(config_wifi_ssid);
	return 0;
}

static rt_bool_t find_ssid(const struct rt_wlan_info *ele)
{
	if (CONFIG_WIFI_PASS == NULL && !is_open(ele->security))
	{
		rt_kprintf("  * %.*s - private: %x, skip", ele->ssid.len, ele->ssid.val, ele->security);
		return RT_FALSE;
	}
	else if (CONFIG_WIFI_PASS != NULL && is_open(ele->security))
	{
		rt_kprintf("  * %.*s - public, skip", ele->ssid.len, ele->ssid.val);
		return RT_FALSE;
	}

	if (str_prefix((char *)ele->ssid.val, sizeof(CONFIG_SERVER_SSID), CONFIG_SERVER_SSID) == 0)
	{
		KPRINTF_DIM("  * %.*s - yes", ele->ssid.len, ele->ssid.val);
		strncpy(config_wifi_ssid, (char *)ele->ssid.val, ele->ssid.len);
		return RT_TRUE;
	}
	rt_kprintf("  * %.*s - not related", ele->ssid.len, ele->ssid.val);
	return RT_FALSE;
}
extern void print_netdev_list();

rt_err_t config_mode_connect_wifi()
{
	// print_netdev_list();

	rt_wlan_set_mode(RT_WLAN_DEVICE_STA_NAME, RT_WLAN_STATION);

	if (config_wifi_ssid == NULL)
		config_wifi_ssid = malloc(MAX_SSID_LENGTH);
	memset(config_wifi_ssid, 0, MAX_SSID_LENGTH);

	while (1)
	{
		wifi_scan_dump(find_ssid);
		if (config_wifi_ssid[0] != '\0')
			break;
		rt_thread_mdelay(3000);
	}

	KPRINTF_COLOR(14, "Connect WiFi: %s", config_wifi_ssid);
	if (rt_wlan_connect(config_wifi_ssid, CONFIG_WIFI_PASS) != RT_EOK)
	{
		KPRINTF_COLOR(9, "wifi connect fail.");
		return RT_ERROR;
	}

	KPRINTF_COLOR(10, "WiFi connected.");

	struct netdev *dev = CONFIG_INTERFACE_NETDEV;
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
		// 	KPRINTF_COLOR(9, "Can not get IP address after 10 seconds.");
		// 	return RT_ETIMEOUT;
		// }
		if (!rt_wlan_is_connected())
		{
			KPRINTF_COLOR(9, "WiFi connection broken before ready.");
			return RT_EIO;
		}
	}

	KPRINTF_COLOR(10, "config WiFi is ready.");
	wifi_status_dump();
	return RT_EOK;
}
