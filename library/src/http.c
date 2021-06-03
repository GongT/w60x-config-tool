#include "private.h"
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdarg.h>

#define HTTP_TRACE if (0)
#define HTTP_TRACE_IO if (0)

static const char *ltrim(const char *str)
{
	while (str[0] == ' ')
	{
		str++;
	}
	return str;
}

static char recv_safe(char **iitr)
{
	char *itr = *iitr;
	if (itr - BUFF >= BUFF_SIZE - 2)
	{
		KPRINTF_COLOR(9, "recv buffer(%d bytes) overflow: itr=0x%X BUF=0x%X", BUFF_SIZE, itr, BUFF);
		return RT_FALSE;
	}

	int ret = recv(_SOC_, itr, 1, 0);
	HTTP_TRACE KPRINTF_DIM("    recv_safe() -> 0x%02x = '%s'", *itr, dump_ascii(*itr));
	if (ret <= 0)
	{
		KPRINTF_COLOR(9, "recv failed: %d", ret);
		return RT_FALSE;
	}
	*iitr = itr + 1;
	return RT_TRUE;
}
static char drop_until(const char *wantList)
{
	HTTP_TRACE
	{
		SETCOLOR_DIM();
		rt_kputs("drop_until(");
		for (int i = 0; i < strlen(wantList); i++)
		{
			rt_kprintf("0x%02X ");
		}
		rt_kputs(")\n");
		RESETCOLOR();
	}

	char b;
	do
	{
		int ret = recv(_SOC_, &b, 1, 0);
		if (ret <= 0)
		{
			KPRINTF_COLOR(9, "recv failed: %d", ret);
			return -1;
		}
	} while (strchr(wantList, b) == NULL);
	return b;
}

/**
 * @return the ending charactor, -1 if error
 */
static char recv_until(const char *wantList)
{
	HTTP_TRACE
	{
		SETCOLOR_DIM();
		rt_kputs("recv_until(");
		for (int i = 0; i < strlen(wantList); i++)
		{
			rt_kprintf("0x%02X ");
		}
		rt_kputs(")\n");
		RESETCOLOR();
	}

	char *next = BUFF, *current = BUFF;
	do
	{
		current = next;
		if (!recv_safe(&next))
		{
			KPRINTF_COLOR(9, "drop_until failed");
			return -1;
		}
		if (*current == '\r')
		{
			HTTP_TRACE KPRINTF_COLOR(8, "    got 0x%02X(\\r), skip", *current);
			next = current;
			*next = -1;
			if (next < BUFF)
				next = BUFF;
			continue;
		}
		HTTP_TRACE
		{
			if (strchr(wantList, *current) == NULL)
				KPRINTF_COLOR(8, "[%d] strchr(..., 0x%02X) = %d", next - 1 - BUFF, *current, strchr(wantList, *current) - wantList);
			else
				KPRINTF_COLOR(2, "[%d] strchr(..., 0x%02X) = %d", next - 1 - BUFF, *current, strchr(wantList, *current) - wantList);
		}
	} while (strchr(wantList, *current) == NULL);
	const char ret = *current;
	*current = '\0';
	HTTP_TRACE KPRINTF_COLOR(10, "recv_until(): replace char 0x%02X with NUL at %d, BUFF is %d bytes", ret, current - BUFF, strlen(BUFF));
	return ret;
}

static void drain()
{
	char c;
	while (recv(_SOC_, &c, 1, MSG_DONTWAIT) > 0)
		;
}

http_response response;

static inline http_response resp_empty(const char *const log, ...)
{
	if (log != NULL)
	{
		va_list argptr;
		va_start(argptr, log);
		rt_vsprintf(BUFF, log, argptr);
		va_end(argptr);

		KPRINTF_COLOR(9, "%s", BUFF);
	}

	drain();
	BUFF_CLR();
	response.ok = RT_TRUE;
	response.error = HTTP_ERR_OK;
	response.size = 0;
	return response;
}

static inline http_response resp_error(enum HTTP_ERROR ERR, const char *const log, ...)
{
	if (log != NULL)
	{
		va_list argptr;
		va_start(argptr, log);
		rt_vsprintf(BUFF, log, argptr);
		va_end(argptr);

		KPRINTF_COLOR(9, "%s", BUFF);
	}

	drain();
	BUFF_CLR();
	response.ok = RT_FALSE;
	response.error = ERR;
	return response;
}

const http_response config_request_data_single(const char *const name, size_t start, size_t length)
{
	HTTP_TRACE KPRINTF_COLOR(5, " - request config: %s [len=%d]", name, strlen(name));
	assert(BUFF != NULL);

	char *itr = BUFF;
	BUFF_WRITE_ITR("GET /%s HTTP/1.1\nHost: _\nContent-Length: 0\nConnection: keep-alive\nKeep-Alive: timeout=5, max=1000\n", name);
	if (length > 0)
		BUFF_WRITE_ITR("Range: bytes=%d-%d\n", start, start + length);
	BUFF_WRITE_ITR("\n");

	HTTP_TRACE_IO KPRINTF_DIM("=========%s=========", BUFF);
	int send_ret = socket_send_str(BUFF);
	if (send_ret <= 0)
		return resp_error(HTTP_ERR_NETWORK, "send failed: %d", send_ret);

	const char nl = recv_until("\n");
	if (nl < 0)
		return resp_error(HTTP_ERR_NETWORK, NULL);

	HTTP_TRACE KPRINTF_DIM("response title: %s", BUFF);
	unsigned long http_code = strtoul(BUFF + strlen("HTTP/1.1 "), NULL, 10);
	response.code = http_code;
	if (http_code != (length > 0 ? 206 : 200))
		return resp_empty("HTTP status not OK: want %s got: %d", length > 0 ? "206" : "200", http_code);

	size_t body_size = 0, range_start = 0, range_end = 0;
	while (1)
	{
		const char got_char = recv_until(":\n");
		switch (got_char)
		{
		case -1:
			return resp_error(HTTP_ERR_NETWORK, "oops, recv_until() should not fail(return -1) here (mostly network broken)...");
		case ':':
			HTTP_TRACE rt_kprintf("%s = ", BUFF);
			if (strcasecmp(BUFF, "content-length") == 0)
			{
				if (recv_until("\n") < 0)
					return resp_error(HTTP_ERR_NETWORK, "");
				HTTP_TRACE KPRINTF_DIM("%s", BUFF);
				body_size = (size_t)strtoul(ltrim(BUFF), NULL, 10);
				HTTP_TRACE_IO rt_kprintf("    content-length (to number) = %s\n", BUFF);
			}
			else if (length > 0 && strcasecmp(BUFF, "content-range") == 0)
			{
				if (recv_until("\n") < 0)
					return resp_error(HTTP_ERR_NETWORK, "");
				HTTP_TRACE rt_kprintf("%s", BUFF);

				const char *inp = ltrim(ltrim(BUFF) + strlen("bytes"));
				char *outp = NULL;
				range_start = (size_t)strtoul(inp, &outp, 10);
				// if (start != range_start)
				// 	return resp_error(HTTP_ERR_PROTOCOL, "range start invalid: get %d != want %d", range_chk, start);
				outp++;
				range_end = range_start + (size_t)strtoul(outp, NULL, 10);
				HTTP_TRACE_IO KPRINTF_COLOR(6, "response range: 0x%X -- 0x%X", range_start, range_end);
			}
			else
			{
				HTTP_TRACE rt_kprintf("ignore");
				if (drop_until("\n") < 0)
					return resp_error(HTTP_ERR_NETWORK, "");
			}
			break;
		case '\n':
			if (strlen(BUFF) == 0)
				goto _header_ending;
			KPRINTF_COLOR(9, "invalid header: <%d>%s", strlen(BUFF), BUFF);
			break;
		default:
			return resp_error(HTTP_ERR_PROTOCOL, "what the hell?? 0x%02X = %c", got_char, got_char);
		}
	}

_header_ending:
	HTTP_TRACE KPRINTF_DIM("header ending, bodysize=%d", body_size);
	if (body_size > 0)
	{
		char *itr = BUFF;
		for (size_t i = body_size; i > 0; i--)
			recv_safe(&itr);
		*itr = '\0';
		HTTP_TRACE KPRINTF_DIM("body: %s", BUFF);
		response.ok = RT_TRUE;
		response.buff = BUFF;
		response.size = body_size;
		return response;
	}
	else
	{
		KPRINTF_COLOR(7, "no Content-Length in header, or length is 0");
		response.ok = RT_TRUE;
		response.size = 0;
		return response;
	}
}
