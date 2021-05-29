#pragma once

#define DBG_TAG "\x1B[1mConfigMode\x1B[22m"
#define DBG_LVL DBG_WARNING
#include <rtdbg.h>

#include <rtthread.h>
#include <rtdevice.h>

#include "gongt/config_tool.h"
#include "../shared/connection_consts.h"

#include <string.h>
__attribute__((always_inline)) inline static int str_prefix(const char *pre, size_t n, const char *str)
{
	return strncmp((char *)pre, (char *)str, n) == 0;
}

enum CONFIG_STATUS config_mode_OTA();

rt_bool_t config_request_data_single(const char *const name, size_t start, size_t length);
const char *config_mode_request_data(const char *const name, const char *const dev_id);
rt_err_t config_mode_connect_tcp();
rt_err_t config_mode_disconnect_tcp();
rt_err_t config_mode_connect_wifi();
rt_err_t config_mode_disconnect_wifi();
const char *dump_ascii(char chr);

extern int config_mode_http_socket;
#define _SOC_ ((const int)config_mode_http_socket)

extern char *config_mode_http_buff;
#define BUFF ((char *const)(config_mode_http_buff + 1))
#define BUFF_SIZE 4096 + 512 + 1
#define BUFF_WRITE(...) rt_snprintf(BUFF, BUFF_SIZE, __VA_ARGS__)
#define BUFF_WRITE_ITR(...) itr += rt_snprintf(itr, BUFF_SIZE - (itr - BUFF), __VA_ARGS__) - 1
#define BUFF_CLR() (BUFF[0] = '\0')

#define socket_send_str(str) send(_SOC_, str, strlen(str), 0)

#ifdef RT_DEBUG_COLOR
#define KPINTF_COLOR(COLOR, MSG, ...) (rt_kprintf("\033[38;5;" #COLOR "m" MSG "\033[0m\n", ##__VA_ARGS__))
#define KPINTF_LIGHT(MSG, ...) (rt_kprintf("\033[1m" MSG "\033[0m\n", ##__VA_ARGS__))
#define KPINTF_DIM(MSG, ...) (rt_kprintf("\033[2m" MSG "\033[0m\n", ##__VA_ARGS__))
#define SETCOLOR_DIM() (rt_kputs("\033[2m"))
#define RESETCOLOR() (rt_kputs("\033[0m"))
#else
#define KPINTF_COLOR(COLOR, MSG, ...) (rt_kprintf(MSG "\n", ##__VA_ARGS__))
#define KPINTF_LIGHT(MSG, ...) (rt_kprintf(MSG "\n", ##__VA_ARGS__))
#define KPINTF_DIM(MSG, ...) (rt_kprintf(MSG "\n", ##__VA_ARGS__))
#define SETCOLOR_DIM() ((void)0)
#define RESETCOLOR() ((void)0)
#endif
