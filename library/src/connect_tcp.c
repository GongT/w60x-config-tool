#include "private.h"
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>

#undef BUFF

int config_mode_http_socket = -1;
char *config_mode_http_buff = NULL;

rt_err_t config_mode_disconnect_tcp()
{
	if (config_mode_http_socket >= 0)
	{
		closesocket(config_mode_http_socket);
		config_mode_http_socket = -1;
	}
	if (config_mode_http_buff != NULL)
	{
		rt_free(config_mode_http_buff);
		config_mode_http_buff = NULL;
	}
	return 0;
}

rt_err_t config_mode_connect_tcp()
{
	if (config_mode_http_socket >= 0 || config_mode_http_buff != NULL)
	{
		return RT_EOK;
	}

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		KPRINTF_COLOR(9, "Socket creation error.");
		return RT_EIO;
	}

	if (config_mode_http_buff == NULL)
	{
		config_mode_http_buff = rt_malloc(BUFF_SIZE);
		if (config_mode_http_buff == NULL)
		{
			KPRINTF_COLOR(9, "no memory for recv buffer.");
			return RT_ENOMEM;
		}
		rt_memset(config_mode_http_buff, 0, BUFF_SIZE);
	}

	// 101a8c0:22680
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(CONFIG_SERVER_IP_PORT);
	inet_pton(AF_INET, CONFIG_SERVER_IP_HOST, &server_addr.sin_addr);
	rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

	KPRINTF_COLOR(14, "connect TCP server %s:%d ...", CONFIG_SERVER_IP_HOST, CONFIG_SERVER_IP_PORT);
	while (connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
	{
		KPRINTF_COLOR(9, "TCP connect fail.");
		if (!rt_wlan_is_ready())
		{
			closesocket(sock);
			KPRINTF_COLOR(9, "wifi connect broken.");
			return RT_EIO;
		}
		rt_thread_mdelay(5000);
	}
	KPRINTF_COLOR(10, "tcp connected.");

	config_mode_http_socket = sock;
	return RT_EOK;
}
