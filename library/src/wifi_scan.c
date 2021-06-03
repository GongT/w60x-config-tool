#include "private.h"
#include <sys/socket.h>
#include <netdev.h>
#include <stdio.h>

int wifi_scan_dump(wifi_scan_element callback)
{
	struct netdev *dev = CONFIG_INTERFACE_NETDEV;
	if (dev == NULL)
	{
#define STR_VALUE(arg) #arg
		KPRINTF_COLOR(9, "can not find netdev: " STR_VALUE(CONFIG_INTERFACE_NETDEV) "\n");
		return -1;
	}

	struct rt_wlan_scan_result *scan_result = rt_wlan_scan_sync();
	if (!scan_result)
	{
		KPRINTF_DIM("  * no result!");
		return 0;
	}
	uint8_t index;
	for (index = 0; index < scan_result->num; index++)
	{
		struct rt_wlan_info *ele = &scan_result->info[index];
		SETCOLOR_DIM();
		rt_kprintf("  * %.*s", ele->ssid.len, ele->ssid.val);
		int quit = (callback != NULL) && callback(ele);
		rt_kputs("\n");
		RESETCOLOR();
		if (quit)
			break;
	}
	rt_wlan_scan_result_clean();
	return index + 1;
}
