#include "private.h"
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>

static char *rtrimnl(char *buff)
{
	char *itr = buff;
	while (*itr != '\n' && *itr != '\r' && *itr != '\0')
		itr++;
	*itr = '\0';
	return buff;
}

const char *config_mode_request_data(const char *const name, const char *const dev_id)
{
	rt_bool_t ret;

	assert(strlen(name) > 0);
	assert(_SOC_ >= 0);

	char name_buff[CONFIG_MAX_KEY_SIZE];

#define __CHECK_RET()                 \
	if (ret == RT_FALSE)              \
	{                                 \
		config_mode_disconnect_tcp(); \
		return NULL;                  \
	}                                 \
	else if (strlen(BUFF) > 0)        \
	return rtrimnl(BUFF)

	rt_snprintf(name_buff, CONFIG_MAX_KEY_SIZE, "%s/%s", dev_id, name);
	ret = config_request_data_single(name_buff, 0, 0);
	__CHECK_RET();

	rt_snprintf(name_buff, CONFIG_MAX_KEY_SIZE, "%s/%s", APPLICATION_KIND, name);
	ret = config_request_data_single(name_buff, 0, 0);
	__CHECK_RET();

	ret = config_request_data_single(name, 0, 0);
	__CHECK_RET();

	return BUFF;
}
