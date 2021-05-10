#pragma once

#if !defined(DBG_TAG) && !defined(DBG_SECTION_NAME)
#warning "此文件没有定义C语言常量DBG_TAG或DBG_SECTION_NAME，默认为unknown"
#define DBG_TAG "main"
#endif
#ifndef DBG_LVL
#define DBG_LVL DBG_WARNING
#endif
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

const char *config_request_data(const char *const name);
rt_err_t config_mode_connect_tcp();
rt_err_t config_mode_connect_wifi();
