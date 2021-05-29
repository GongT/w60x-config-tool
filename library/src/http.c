#include "private.h"
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>

#define HTTP_TRACE if (0)

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
		KPINTF_COLOR(9, "recv buffer overflow");
		return RT_FALSE;
	}

	int ret = recv(_SOC_, itr, 1, 0);
	HTTP_TRACE KPINTF_DIM("    recv_safe() -> 0x%02x = '%s'", *itr, dump_ascii(*itr));
	if (ret <= 0)
	{
		KPINTF_COLOR(9, "recv failed: %d", ret);
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
		char *bp = &b;
		if (!recv_safe(&bp))
			return -1;
	} while (strchr(wantList, b) == NULL);
	return b;
}

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
			return -1;
		if (*current == '\r')
		{
			HTTP_TRACE KPINTF_COLOR(8, "    got 0x%02X(\\r), skip", *current);
			next = current;
			*next = -1;
			if (next < BUFF)
				next = BUFF;
			continue;
		}
		HTTP_TRACE
		{
			if (strchr(wantList, *current) == NULL)
				KPINTF_COLOR(8, "[%d] strchr(..., 0x%02X) = %d", next - 1 - BUFF, *current, strchr(wantList, *current) - wantList);
			else
				KPINTF_COLOR(2, "[%d] strchr(..., 0x%02X) = %d", next - 1 - BUFF, *current, strchr(wantList, *current) - wantList);
		}
	} while (strchr(wantList, *current) == NULL);
	const char ret = *current;
	*current = '\0';
	HTTP_TRACE KPINTF_COLOR(10, "recv_until(): replace char 0x%02X with NUL at %d, BUFF is %d bytes", ret, current - BUFF, strlen(BUFF));
	return ret;
}

static void drain()
{
	char c;
	while (recv(_SOC_, &c, 1, MSG_DONTWAIT) > 0)
		;
}

rt_bool_t config_request_data_single(const char *const name, size_t start, size_t length)
{
	int ret;
#define return_EMPTY() \
	drain();           \
	BUFF_CLR();        \
	return RT_TRUE;

#define return_ERROR(...)             \
	{                                 \
		KPINTF_COLOR(9, __VA_ARGS__); \
		return RT_FALSE;              \
	}

	HTTP_TRACE KPINTF_COLOR(5, " - request config: %s [len=%d]", name, strlen(name));
	assert(BUFF != NULL);

	char *itr = BUFF;
	BUFF_WRITE_ITR("GET /%s HTTP/1.1\nHost: _\nContent-Length: 0\nConnection: keep-alive\nKeep-Alive: timeout=5, max=1000\n", name);
	if (length > 0)
		BUFF_WRITE_ITR("Range: bytes=%d-%d\n", name, start, start + length);
	BUFF_WRITE_ITR("\n\n");

	HTTP_TRACE KPINTF_DIM("=========%s=========", BUFF);
	if ((ret = socket_send_str(BUFF)) <= 0)
		return_ERROR("send failed: %d", ret);

	const char nl = recv_until("\n");
	if (nl < 0)
		return RT_FALSE;

	HTTP_TRACE KPINTF_DIM("response title: %s", BUFF);
	unsigned long http_code = strtoul(BUFF + strlen("HTTP/1.1 "), NULL, 10);

	if (http_code != (length > 0 ? 206 : 200))
	{
		KPINTF_DIM("HTTP status not OK: want %s got: %d", length > 0 ? "206" : "200", http_code);
		return_EMPTY();
	}

	size_t body_size = 0;
	while (1)
	{
		const char got_char = recv_until(":\n");
		switch (got_char)
		{
		case -1:
			KPINTF_COLOR(9, "oops, recv_until() should not fail(return -1) here (mostly network broken)...");
			return RT_FALSE;
		case ':':
			HTTP_TRACE rt_kprintf("\x1B[2m%s = ", BUFF);
			if (strcasecmp(BUFF, "content-length") == 0)
			{
				if (recv_until("\n") < 0)
					return RT_FALSE;
				HTTP_TRACE KPINTF_DIM("%s", BUFF);
				body_size = (size_t)strtoul(ltrim(BUFF), NULL, 10);
				HTTP_TRACE KPINTF_DIM("    content-length (to number) = %s", BUFF);
			}
			else if (length > 0 && strcasecmp(BUFF, "content-range") == 0)
			{
				if (recv_until("\n") < 0)
					return RT_FALSE;
				HTTP_TRACE KPINTF_DIM("%s", BUFF);

				const char *inp = ltrim(ltrim(BUFF) + strlen("bytes"));
				char *outp = NULL;
				size_t range_chk = (size_t)strtoul(inp, &outp, 10);
				if (start != range_chk)
				{
					KPINTF_COLOR(9, "range start invalid: get %d != want %d", range_chk, start);
					return_EMPTY();
				}
				outp++;
				range_chk = (size_t)strtoul(outp, NULL, 10);
				if (start + length != range_chk)
				{
					KPINTF_COLOR(9, "range end invalid: get %d != want %d", range_chk, start + length);
					return_EMPTY();
				}
				HTTP_TRACE KPINTF_DIM("response range valid!");
			}
			else
			{
				HTTP_TRACE KPINTF_DIM("ignore");
				if (drop_until("\n") < 0)
					return RT_FALSE;
			}
			break;
		case '\n':
			if (strlen(BUFF) == 0)
				goto _header_ending;
			KPINTF_COLOR(9, "invalid header: <%d>%s", strlen(BUFF), BUFF);
			break;
		default:
			KPINTF_COLOR(9, "what the hell?? 0x%02X = %c", got_char, got_char);
			return RT_FALSE;
		}
	}

_header_ending:
	HTTP_TRACE KPINTF_DIM("header ending, bodysize=%d", body_size);
	if (body_size > 0)
	{
		char *itr = BUFF;
		for (size_t i = body_size; i > 0; i--)
			recv_safe(&itr);
		*itr = '\0';
		HTTP_TRACE KPINTF_DIM("body: %s", BUFF);
		return RT_TRUE;
	}
	else
	{
		KPINTF_COLOR(9, "no Content-Length in header, or length is 0");
		return_EMPTY();
	}
}
