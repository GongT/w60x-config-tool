#define DBG_TAG "main"

#include "app.h"

int main()
{
	rt_thread_t thread = rt_thread_create("serv", start_tcp_server, NULL, 8192, 10, 10);
	rt_thread_startup(thread);

	LOG_I("Main return");
	return 0;
}
