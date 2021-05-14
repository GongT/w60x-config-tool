#pragma once

#include <rtdef.h>

enum CONFIG_STATUS
{
	CONFIG_STATUS_SUCCESS = 0,
	CONFIG_STATUS_INIT,
	CONFIG_STATUS_WIFI_FAIL,
	CONFIG_STATUS_TCP_FAIL,
	CONFIG_STATUS_ITEM_MISSING,
	CONFIG_STATUS_STORAGE_FAIL,
};

// you MUST define this somewhere
extern const char *const config_names[];
extern rt_err_t save_config_item(const char *config_name, const char *value);
// |END| you MUST define this somewhere

#define FOREACH_CONFIG(PTR_NAME)            \
	const char *PTR_NAME = config_names[0]; \
	for (const char *const *__ptr_element = config_names; PTR_NAME != NULL; __ptr_element++, PTR_NAME = *__ptr_element)

enum CONFIG_STATUS goto_config_mode();
void wifi_status_dump();

#define CONFIG_END_CLOSE "{__end__}"
#define END_CONFIG CONFIG_END_CLOSE, NULL

#define DEFINE_CONFIG_NAMES(...) const char *const config_names[] = { \
									 __VA_ARGS__                      \
										 END_CONFIG}
