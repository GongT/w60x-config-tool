#pragma once

#if !defined(DBG_TAG) && !defined(DBG_SECTION_NAME)
#warning "此文件没有定义C语言常量DBG_TAG或DBG_SECTION_NAME，默认为unknown"
#define DBG_TAG "main"
#endif
#ifndef DBG_LVL
#define DBG_LVL DBG_LOG
#endif
#include <rtdbg.h>

#include <string.h>

#define ALWAYS_INLINE __attribute__((always_inline)) inline static

ALWAYS_INLINE int str_prefix(const char *pre, size_t n, const char *str)
{
	return strncmp(pre, str, n) == 0;
}

ALWAYS_INLINE int str_eq(const char *str, const char *cmp)
{
	return strcmp(str, cmp) == 0;
}

#define assert0(V) RT_ASSERT(V == 0)

// 输出错误
#define FATAL_ERROR(...) __FATAL_ERROR(__FUNCTION__, __LINE__, __VA_ARGS__)
__attribute__((noreturn)) void __FATAL_ERROR(const char *const MSG, ...);
