#include "private.h"
#include <rthw.h>
#include <wm_internal_flash.h>

static void dump_boot_header(const T_BOOTER *header)
{
	KPRINTF_DIM("magic_no          = 0x%08X", header->magic_no);
	KPRINTF_DIM("img_type          = 0x%04X", header->img_type);
	KPRINTF_DIM("zip_type          = 0x%04X", header->zip_type);
	KPRINTF_DIM("run_img_addr      = 0x%08X", header->run_img_addr);
	KPRINTF_DIM("run_img_len       = 0x%08X", header->run_img_len);
	KPRINTF_DIM("run_org_checksum  = 0x%08X", header->run_org_checksum);
	KPRINTF_DIM("upd_img_addr      = 0x%08X", header->upd_img_addr);
	KPRINTF_DIM("upd_img_len       = 0x%08X", header->upd_img_len);
	KPRINTF_DIM("upd_checksum      = 0x%08X", header->upd_checksum);
	KPRINTF_DIM("upd_no            = 0x%08X", header->upd_no);
	KPRINTF_DIM("ver               = %.16s", header->ver);
	KPRINTF_DIM("hd_checksum       = 0x%08X", header->hd_checksum);
}
static rt_bool_t write_update(T_BOOTER *header)
{
	config_mode_status_callback(CONFIG_STATUS_FLASH_WRITE);
	KPRINTF_DIM("write_update: tls_fls_write(0x%X, ..., %d)", CODE_UPD_HEADER_ADDR, sizeof(T_BOOTER));
	int ret = tls_fls_write(CODE_UPD_HEADER_ADDR, (void *)header, sizeof(T_BOOTER));
	if (ret != 0)
	{
		KPRINTF_COLOR(9, "write_update: tls_fls_write(0x%X, ..., %d) return %d", CODE_UPD_HEADER_ADDR, sizeof(T_BOOTER), ret);
		return RT_FALSE;
	}
	return RT_TRUE;
}

static rt_bool_t write(size_t offset, size_t size, const char *data)
{
	config_mode_status_callback(CONFIG_STATUS_FLASH_WRITE);
	// 0x8090000
	size_t start = CODE_UPD_START_ADDR + offset;
	if (start + size > USER_ADDR_START)
	{
		KPRINTF_COLOR(9, "update image overflow (+0x%X + 0x%X) 0x%X > 0x%X", offset, size, start + size, USER_ADDR_START);
		return RT_FALSE;
	}
	KPRINTF_DIM("write: tls_fls_write(0x%X, ..., %d)", start, size);
	int ret = tls_fls_write(start, (void *)data, size);
	if (ret != 0)
	{
		KPRINTF_COLOR(9, "write: tls_fls_write(0x%X, ..., %d) return %d", start, size, ret);
		return RT_FALSE;
	}
	return RT_TRUE;
}

enum CONFIG_STATUS config_mode_OTA()
{
	config_mode_status_callback(CONFIG_STATUS_OTA_START);
	T_BOOTER current_header;
	tls_fls_read(CODE_UPD_HEADER_ADDR, (void *)&current_header, sizeof(T_BOOTER));
	rt_kprintf("current update header: (0x%X)\n", CODE_UPD_HEADER_ADDR);
	dump_boot_header(&current_header);

	tls_fls_read(CODE_RUN_HEADER_ADDR, (void *)&current_header, sizeof(T_BOOTER));
	rt_kprintf("current run header: (0x%X)\n", CODE_RUN_HEADER_ADDR);

	dump_boot_header(&current_header);
	char *url = malloc(256);
	snprintf(url, 256, APPLICATION_KIND "/application.img?current=%.16s@0x%08X", current_header.ver, current_header.upd_checksum);
	http_response resp = config_request_data_single(url, 0, sizeof(T_BOOTER));
	if (!resp.ok)
	{
		KPRINTF_COLOR(6, "OTA header return failed.");
		return CONFIG_STATUS_HTTP_FAIL;
	}
	if (resp.code == 404)
	{
		KPRINTF_COLOR(6, "OTA package seems not exists.");
		return CONFIG_STATUS_ITEM_MISSING;
	}

	T_BOOTER header;
	memcpy(&header, resp.buff, resp.size);

	rt_kprintf("receive header:\n");
	dump_boot_header(&header);

	if (update_img_header_check(&header) == RT_FALSE)
	{
		KPRINTF_COLOR(9, "input header is invalid");
		return CONFIG_STATUS_SERVER;
	}

	if (header.run_org_checksum == current_header.run_org_checksum)
	{
		KPRINTF_COLOR(10, "application is up to date");
		config_mode_status_callback(CONFIG_STATUS_OTA_END);
		return CONFIG_STATUS_SUCCESS;
	}
	config_mode_status_callback(CONFIG_STATUS_OTA_NEW);

	size_t header_size = resp.size;
	size_t current = 0;
	while (1)
	{
		config_mode_status_callback(CONFIG_STATUS_HTTP_SEND);
		resp = config_request_data_single(APPLICATION_KIND "/application.img", current + header_size, INSIDE_FLS_SECTOR_SIZE);
		if (!resp.ok)
		{
			KPRINTF_COLOR(9, "failed read block %d", current / INSIDE_FLS_SECTOR_SIZE);
			return CONFIG_STATUS_HTTP_FAIL;
		}
		if (resp.size != 0)
		{
			if (resp.size > INSIDE_FLS_SECTOR_SIZE)
			{
				KPRINTF_COLOR(9, "invalid return length = %d, must be %d.", resp.size, INSIDE_FLS_SECTOR_SIZE);
				return CONFIG_STATUS_SERVER;
			}
			if (!write(current, INSIDE_FLS_SECTOR_SIZE, resp.buff))
			{
				KPRINTF_COLOR(9, "failed write block %d", current / INSIDE_FLS_SECTOR_SIZE);
				return CONFIG_STATUS_STORAGE_FAIL;
			}
		}

		current += resp.size;
		if (resp.size < INSIDE_FLS_SECTOR_SIZE)
		{
			KPRINTF_COLOR(10, "update image write complete.");
			break;
		}

		// led_static(LED_RED, led_state = !led_state);
		rt_thread_yield();
	}

	if (!write_update(&header))
		return CONFIG_STATUS_STORAGE_FAIL;
	KPRINTF_COLOR(10, "write update header success.");

	config_mode_status_callback(CONFIG_STATUS_OTA_END);
	return CONFIG_STATUS_REBOOT_REQUIRED;
}
// 175
// 313

// =138

// 54000
