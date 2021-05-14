#define DBG_TAG "CFG:TCP"

#include "private.h"
#include <sys/socket.h>
#include <netdb.h>

static int config_socket = -1;

rt_err_t config_mode_disconnect_tcp()
{
	if (config_socket >= 0)
	{
		closesocket(config_socket);
		config_socket = -1;
	}
}

rt_err_t config_mode_connect_tcp()
{
	if (config_socket >= 0)
	{
		return config_socket;
	}

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		LOG_E("Socket creation error.");
		return RT_ERROR;
	}

	// 101a8c0:22680
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(CONFIG_SERVER_IP_PORT);
	inet_pton(AF_INET, CONFIG_SERVER_IP_HOST, &server_addr.sin_addr);
	rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

	LOG_I("connect TCP server...", server_addr.sin_addr.s_addr, server_addr.sin_port);
	while (connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
	{
		LOG_W("Connect fail.");
		if (!rt_wlan_is_ready())
		{
			closesocket(sock);
			LOG_E("wifi connect broken.");
			return RT_EIO;
		}
		rt_thread_mdelay(5000);
	}
	LOG_I("connected.");

	config_socket = sock;
	return RT_EOK;
}

#define BUFF_SIZE 4096

const char *config_request_data(const char *const name)
{
	static char *buff = NULL;
	if (buff == NULL)
	{
		buff = rt_malloc(BUFF_SIZE);
		if (buff == NULL)
		{
			LOG_E("no memory for recv buffer.");
			return NULL;
		}
	}
	rt_memset(buff, 0, BUFF_SIZE);

	size_t name_len = strlen(name);

	assert(name_len > 0);

	int ret = send(config_socket, name, name_len + 1, 0);
	if (ret <= 0)
	{
		LOG_E("send failed: %d", ret);
		return NULL;
	}

	char *itr = buff;
	while (itr - buff < BUFF_SIZE)
	{
		ret = recv(config_socket, itr, 1, 0);
		if (ret <= 0)
		{
			LOG_E("recv failed: %d", ret);
			return NULL;
		}
		LOG_I("recv: %c | %s", *itr, buff);

		if (*itr == '\0')
		{
			LOG_I("recv: %c | %s", buff);
			return buff;
		}

		itr++;
	}

	LOG_E("recv buffer overflow!");
	return NULL;
}
