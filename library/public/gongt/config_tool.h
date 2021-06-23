#pragma once

#include <rtdef.h>

enum CONFIG_STATUS
{
	CONFIG_STATUS_SUCCESS = 0,
	CONFIG_STATUS_REBOOT_REQUIRED,
	CONFIG_STATUS_INIT,
	CONFIG_STATUS_WIFI_FAIL,
	CONFIG_STATUS_TCP_FAIL,
	CONFIG_STATUS_NO_MAC,
	CONFIG_STATUS_HTTP_FAIL,
	CONFIG_STATUS_SERVER,
	CONFIG_STATUS_ITEM_MISSING,
	CONFIG_STATUS_STORAGE_FAIL,
	// virtual states
	CONFIG_STATUS_WIFI_CONNECT,
	CONFIG_STATUS_HTTP_SEND,
	CONFIG_STATUS_FLASH_WRITE,
	CONFIG_STATUS_OTA_START,
	CONFIG_STATUS_OTA_NEW,
	CONFIG_STATUS_OTA_END,
	CONFIG_STATUS_CONFIG_START,
	CONFIG_STATUS_CONFIG_END,
};

// you MUST define this somewhere
extern const char *const config_names[];
extern rt_err_t save_config_item(const char *config_name, const char *value);
// |END| you MUST define this somewhere

extern void config_mode_status_callback(enum CONFIG_STATUS status);

#define FOREACH_CONFIG(PTR_NAME)            \
	const char *PTR_NAME = config_names[0]; \
	for (const char *const *__ptr_element = config_names; PTR_NAME != NULL; __ptr_element++, PTR_NAME = *__ptr_element)

enum CONFIG_STATUS goto_config_mode();
enum CONFIG_STATUS goto_config_mode_OTA();
// enum CONFIG_STATUS goto_config_mode_OTA();
void wifi_status_dump();
typedef rt_bool_t (*wifi_scan_element)(const struct rt_wlan_info *);
int wifi_scan_dump(wifi_scan_element callback);

#define END_CONFIG NULL

#define DEFINE_CONFIG_NAMES(...) const char *const config_names[] = { \
									 __VA_ARGS__                      \
										 END_CONFIG}
#include "rtconfig.h"
#ifndef APPLICATION_KIND
#error "必须在rtconfig_project.h中定义APPLICATION_KIND"
#endif
