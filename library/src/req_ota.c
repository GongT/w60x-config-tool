#include "private.h"
#include <rthw.h>
#include <wm_fwup.h>
#include <wm_internal_flash.h>

static void dump_boot_header(const T_BOOTER *header)
{
	KPINTF_DIM("magic_no          = 0x%08X", header->magic_no);
	KPINTF_DIM("img_type          = 0x%04X", header->img_type);
	KPINTF_DIM("zip_type          = 0x%04X", header->zip_type);
	KPINTF_DIM("run_img_addr      = 0x%08X", header->run_img_addr);
	KPINTF_DIM("run_img_len       = 0x%08X", header->run_img_len);
	KPINTF_DIM("run_org_checksum  = 0x%08X", header->run_org_checksum);
	KPINTF_DIM("upd_img_len       = 0x%08X", header->upd_img_len);
	KPINTF_DIM("upd_checksum      = 0x%08X", header->upd_checksum);
	KPINTF_DIM("upd_no            = 0x%08X", header->upd_no);
	KPINTF_DIM("ver               = %.16s", header->ver);
	KPINTF_DIM("hd_checksum       = 0x%08X", header->hd_checksum);
}

enum CONFIG_STATUS config_mode_OTA()
{
	T_BOOTER current_header;
	tls_fls_read(CODE_UPD_HEADER_ADDR, (void *)&current_header, sizeof(T_BOOTER));

	rt_kprintf("current header:\n");
	dump_boot_header(&current_header);

	const char *header_string = config_request_data_single(APPLICATION_KIND "/application.img", 0, 8 * 14);
	if (header_string == NULL)
	{
		KPINTF_COLOR(7, "can not get OTA version.");
		return CONFIG_STATUS_SUCCESS;
	}

	T_BOOTER header;
	uint32_t *itr = (void *)&header;
	size_t s = strlen(header_string);

	if (s % 4 != 0)
	{
		KPINTF_COLOR(7, "OTA header length (%d) is invalid.", s);
		return CONFIG_STATUS_SUCCESS;
	}

	char temp[17];
	for (size_t i = 0; i < s; i += 4, itr++)
	{
		rt_snprintf(temp, 17, header_string + i);
		*itr = (uint32_t)strtoul(temp, NULL, 16);
	}

	rt_kprintf("receive header:\n");
	dump_boot_header(&header);

	if (!tls_fwup_img_header_check(&header))
	{
		KPINTF_COLOR(9, "input header is invalid");
		return CONFIG_STATUS_SUCCESS;
	}

	int ret = tls_fwup_init();
	if (ret != TLS_FWUP_STATUS_OK)
	{
		KPINTF_COLOR(9, "failed start fwup: %d", ret);
		return CONFIG_STATUS_SUCCESS;
	}
	u32 session_id = tls_fwup_enter(TLS_FWUP_IMAGE_SRC_WEB);
	if (session_id == 0)
	{
		KPINTF_COLOR(9, "failed enter fwup");
		return CONFIG_STATUS_SUCCESS;
	}

	return CONFIG_STATUS_SUCCESS;
}
