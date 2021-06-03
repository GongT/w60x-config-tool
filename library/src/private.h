#pragma once

#include <rtconfig.h>
#include <wm_internal_flash.h>
#include <rtthread.h>
#include <rtdevice.h>

#include "gongt/config_tool.h"
#include "../shared/connection_consts.h"

#include <rt-thread-w60x/internal-flash.h>

#include <string.h>
__attribute__((always_inline)) inline static int str_prefix(const char *pre, size_t n, const char *str)
{
	return strncmp((char *)pre, (char *)str, n) == 0;
}

enum CONFIG_STATUS config_mode_OTA();

enum HTTP_ERROR
{
	HTTP_ERR_OK = 0,
	HTTP_ERR_NETWORK,
	HTTP_ERR_PROTOCOL,
};
typedef struct HTTP_REQUEST_RESULT
{
	rt_bool_t ok;
	int code;
	enum HTTP_ERROR error;
	size_t size;
	char *buff;
} http_response;

const http_response config_request_data_single(const char *const name, size_t start, size_t length);
const char *config_mode_request_data(const char *const name, const char *const dev_id);
rt_err_t config_mode_connect_tcp();
rt_err_t config_mode_disconnect_tcp();
rt_err_t config_mode_connect_wifi();
rt_err_t config_mode_disconnect_wifi();
#include <wm_fwup.h>
rt_bool_t update_img_header_check(const T_BOOTER *header);
const char *dump_ascii(char chr);

extern int config_mode_http_socket;
#define _SOC_ ((const int)config_mode_http_socket)

extern char *config_mode_http_buff;
#define BUFF ((char *const)(config_mode_http_buff + 1))
#define BUFF_SIZE INSIDE_FLS_SECTOR_SIZE + 512 + 1
#define BUFF_WRITE(...) rt_snprintf(BUFF, BUFF_SIZE, __VA_ARGS__)
#define BUFF_WRITE_ITR(...) itr += rt_snprintf(itr, BUFF_SIZE - (itr - BUFF) - 1, __VA_ARGS__)
#define BUFF_CLR() (BUFF[0] = '\0')

#define socket_send_str(str) send(_SOC_, str, strlen(str), 0)

#ifdef RT_DEBUG_COLOR
#define KPRINTF_COLOR(COLOR, MSG, ...) (rt_kprintf("\033[38;5;" #COLOR "m" MSG "\033[0m\n", ##__VA_ARGS__))
#define KPRINTF_LIGHT(MSG, ...) (rt_kprintf("\033[1m" MSG "\033[0m\n", ##__VA_ARGS__))
#define KPRINTF_DIM(MSG, ...) (rt_kprintf("\033[2m" MSG "\033[0m\n", ##__VA_ARGS__))
#define SETCOLOR_DIM() (rt_kputs("\033[2m"))
#define RESETCOLOR() (rt_kputs("\033[0m"))
#else
#define KPRINTF_COLOR(COLOR, MSG, ...) (rt_kprintf(MSG "\n", ##__VA_ARGS__))
#define KPRINTF_LIGHT(MSG, ...) (rt_kprintf(MSG "\n", ##__VA_ARGS__))
#define KPRINTF_DIM(MSG, ...) (rt_kprintf(MSG "\n", ##__VA_ARGS__))
#define SETCOLOR_DIM() ((void)0)
#define RESETCOLOR() ((void)0)
#endif
