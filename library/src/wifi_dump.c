#define DBG_SECTION_NAME "wifi"

#include "private.h"
#include <stdio.h>

#define ECHO(f, ...) \
	printf(f "\n", ##__VA_ARGS__)

#define BECHO(f, ...)              \
	printf(f "\n", ##__VA_ARGS__); \
	break

void wifi_status_dump()
{
	struct rt_wlan_info info;
	rt_memset(&info, 0, sizeof(struct rt_wlan_info));
	rt_wlan_get_info(&info);

	ECHO("======================");
	ECHO("station information:");
	switch (info.security)
	{
	case SECURITY_OPEN:
		BECHO("Security: open");
	case SECURITY_WEP_PSK:
		BECHO("Security: wep psk");
	case SECURITY_WEP_SHARED:
		BECHO("Security: wep shared");
	case SECURITY_WPA_TKIP_PSK:
		BECHO("Security: wpa tkip psk");
	case SECURITY_WPA_AES_PSK:
		BECHO("Security: wpa aes psk");
	case SECURITY_WPA2_AES_PSK:
		BECHO("Security: wpa2 aes psk");
	case SECURITY_WPA2_TKIP_PSK:
		BECHO("Security: wpa2 tkip psk");
	case SECURITY_WPA2_MIXED_PSK:
		BECHO("Security: wpa2 mixed psk");
	case SECURITY_WPS_OPEN:
		BECHO("Security: wps open");
	case SECURITY_WPS_SECURE:
		BECHO("Security: wps secure");
	default:
		BECHO("Security: unknown");
	}

	switch (info.band)
	{
	case RT_802_11_BAND_5GHZ:
		BECHO("Band: 5g");
	case RT_802_11_BAND_2_4GHZ:
		BECHO("Band: 2.4g");
	default:
		BECHO("Band: ?");
	}

	ECHO("Data Rate: %dMbps", info.datarate / 1000000);
	ECHO("Radio Channel: %d", info.channel);
	ECHO("Signal Strength: %d", info.rssi);
	ECHO("SSID: %.*s", info.ssid.len, info.ssid.val);
	ECHO("BSSID(MAC): %02x:%02x:%02x:%02x:%02x:%02x", info.bssid[0], info.bssid[1], info.bssid[2], info.bssid[3], info.bssid[4], info.bssid[5]);
	ECHO("Hidden: %d", info.hidden);
	ECHO("======================");
}
