#define DBG_TAG "TCP"
#include "app.h"

#include "connection_consts.h"
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include "easyflash-extend.h"

#define CONFIG_END_CLOSE "{__end__}"

static rt_bool_t send_string(int client, const char *value)
{
	LOG_I("send %d bytes string [%s] to client", strlen(value), value);
	return send(client, value, strlen(value) + 1, 0) > 0;
}

void start_tcp_server()
{
	LOG_I("-- start_tcp_server() --");
	int server = socket(AF_INET, SOCK_STREAM, 0);
	if (server == -1)
	{
		FATAL_ERROR("failed create tcp socket");
	}

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(CONFIG_SERVER_IP_PORT);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

	if (bind(server, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
	{
		FATAL_ERROR("failed socket bind()");
	}

	if (listen(server, 5) == -1)
	{
		FATAL_ERROR("failed socket listen()");
	}

	LOG_I("tcpserver is running: %d", CONFIG_SERVER_IP_PORT);

#define ERR_CLOSE(...)   \
	LOG_W(__VA_ARGS__);  \
	closesocket(client); \
	break;

#define WARN_NEXT(...)       \
	LOG_W(__VA_ARGS__);      \
	send_string(client, ""); \
	continue;

	static char key_buff[CONFIG_MAX_KEY_SIZE + 1] = "";
	static char value_buff[CONFIG_MAX_VALUE_SIZE + 1] = "";
	while (1)
	{
		struct sockaddr_in client_addr;
		unsigned long sin_size = sizeof(struct sockaddr_in);

		int client = accept(server, (struct sockaddr *)&client_addr, &sin_size);
		if (client == -1)
			FATAL_ERROR("server accept() failed.");

		LOG_I("new client: %s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

		while (1)
		{
			int got_size = recv(client, key_buff, CONFIG_MAX_KEY_SIZE, 0);
			if (got_size <= 0)
				ERR_CLOSE("recv failed.");

			if (key_buff[got_size] != '\0')
				ERR_CLOSE("recv data invalid.");

			LOG_I("client request: %s", key_buff);
			if (strcmp(key_buff, CONFIG_END_CLOSE) == 0)
			{
				closesocket(client);
				break;
			}

			size_t v_size = get_storage_size(key_buff);
			if (v_size <= 0)
				WARN_NEXT("missing config!");

			rt_memset(value_buff, 0, CONFIG_MAX_VALUE_SIZE);
			ef_get_env_blob(key_buff, value_buff, CONFIG_MAX_VALUE_SIZE, NULL);

			LOG_I("config value is \"%s\"", value_buff);
			send_string(client, value_buff);
		}
	}
}
