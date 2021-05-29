#define DBG_TAG "TCP:C"
#include "app.h"

#include "connection_consts.h"
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include "easyflash-extend.h"

#ifdef RT_DEBUG_COLOR
#define KPINTF_COLOR(COLOR, MSG, ...) rt_kprintf("\033[38;5;" #COLOR "m" MSG "\033[0m\n", ##__VA_ARGS__)
#define KPINTF_LIGHT(MSG, ...) rt_kprintf("\033[1m" MSG "\033[0m\n", ##__VA_ARGS__)
#define KPINTF_DIM(MSG, ...) rt_kprintf("\033[2m" MSG "\033[0m\n", ##__VA_ARGS__)
#else
#define KPINTF_COLOR(COLOR, MSG, ...) rt_kprintf(MSG "\n", ##__VA_ARGS__)
#define KPINTF_LIGHT(MSG, ...) rt_kprintf(MSG "\n", ##__VA_ARGS__)
#define KPINTF_DIM(MSG, ...) rt_kprintf(MSG "\n", ##__VA_ARGS__)
#endif

#define ERR_CLOSE(...)            \
	KPINTF_COLOR(9, __VA_ARGS__); \
	closesocket(client);          \
	break;

#define WARN_NEXT(...)             \
	KPINTF_COLOR(11, __VA_ARGS__); \
	send_string(client, "");       \
	continue;

static rt_bool_t send_string(int client, const char *value)
{
	KPINTF_DIM("send %d bytes string [%s] to client", strlen(value), value);
	return send(client, value, strlen(value) + 1, 0) > 0;
}

void tcp_thread(void *arg)
{
	char key_buff[CONFIG_MAX_KEY_SIZE + 1] = "";
	char value_buff[CONFIG_MAX_VALUE_SIZE + 1] = "";
	int client = (int)arg;

	while (1)
	{
		int got_size = recv(client, key_buff, CONFIG_MAX_KEY_SIZE, 0);
		if (got_size <= 0)
			ERR_CLOSE("recv failed.");

		if (key_buff[got_size] != '\0')
			ERR_CLOSE("recv data invalid.");

		if (strlen(key_buff) == 0)
		{
			KPINTF_COLOR(10, "client done!\n");
			closesocket(client);
			break;
		}

		KPINTF_DIM("client request: %s", key_buff);
		size_t v_size = get_storage_size(key_buff);
		if (v_size <= 0)
			WARN_NEXT("missing config!");

		rt_memset(value_buff, 0, CONFIG_MAX_VALUE_SIZE);
		ef_get_env_blob(key_buff, value_buff, CONFIG_MAX_VALUE_SIZE, NULL);

		KPINTF_DIM("config value is \"%s\"", value_buff);
		send_string(client, value_buff);
	}
}
