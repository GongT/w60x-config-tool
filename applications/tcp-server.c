#define DBG_TAG "TCP"
#include "app.h"

#include "connection_consts.h"
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>

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

	while (1)
	{
		struct sockaddr_in client_addr;
		unsigned long sin_size = sizeof(struct sockaddr_in);

		int client = accept(server, (struct sockaddr *)&client_addr, &sin_size);
		if (client == -1)
			FATAL_ERROR("server accept() failed.");

		LOG_I("new client: %s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		char cid[10];
		snprintf(cid, 10, "s.%u", client_addr.sin_port);
		rt_thread_startup(rt_thread_create(cid, tcp_thread, (void *)client, 2048, 5, 10));
	}
}
