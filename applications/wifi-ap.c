#define DBG_TAG "WIFI"
#include "app.h"
#include "connection_consts.h"
#include <arpa/inet.h>
#include <netdev.h>
#include <stdio.h>

static void handle_wifi_start(int event, struct rt_wlan_buff *buff, void *parameter)
{
	LOG_I("AP started");
}
static void handle_wifi_connect(int event, struct rt_wlan_buff *buff, void *parameter)
{
	LOG_I("station connect.");
}
static void handle_wifi_disconnect(int event, struct rt_wlan_buff *buff, void *parameter)
{
	LOG_I("station disconnect.");
}

static int start_wifi_ap()
{
	LOG_I("-- start_wifi_ap() --");
	rt_wlan_register_event_handler(RT_WLAN_EVT_AP_START, handle_wifi_start, RT_NULL);
	rt_wlan_register_event_handler(RT_WLAN_EVT_AP_ASSOCIATED, handle_wifi_connect, RT_NULL);
	rt_wlan_register_event_handler(RT_WLAN_EVT_AP_DISASSOCIATED, handle_wifi_disconnect, RT_NULL);

	rt_wlan_set_mode(RT_WLAN_DEVICE_AP_NAME, RT_WLAN_AP);

	struct netdev *dev = netdev_get_by_name("w0");
	while (dev == NULL)
	{
		dev = netdev_get_by_name(RT_WLAN_DEVICE_AP_NAME);
		printf("can not find netdev name: " RT_WLAN_DEVICE_AP_NAME "\n");
		rt_thread_mdelay(5000);
	}

	char name[strlen(CONFIG_SERVER_SSID) + dev->hwaddr_len * 2 + 1 + 1];
	rt_strncpy(name, CONFIG_SERVER_SSID, strlen(CONFIG_SERVER_SSID));

	char *itr = name + strlen(CONFIG_SERVER_SSID);
	*itr = '-';
	itr++;

	for (int i = 0; i < dev->hwaddr_len; i++)
	{
		sprintf(itr, "%02X", dev->hwaddr[i]);
		itr += 2;
	}

	assert(strlen(name) == (unsigned int)(itr - name));

	LOG_I("AP SSID: %.*s", itr - name, name);

	rt_err_t ret;
	if ((ret = rt_wlan_start_ap(name, NULL)) != RT_EOK)
	{
		FATAL_ERROR("Failed start AP [err %d]", ret);
	}
	return 0;
}
INIT_ENV_EXPORT(start_wifi_ap);
