#define DBG_TAG "main"

#include "app.h"

int main()
{
	start_tcp_server();
	LOG_I("Main return");
	return 0;
}
