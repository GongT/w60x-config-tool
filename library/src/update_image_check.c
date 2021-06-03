#include "private.h"
#include <wm_internal_flash.h>
#include <wm_crypto_hard.h>

#define RETURN(...)                      \
	{                                    \
		KPRINTF_COLOR(9, ##__VA_ARGS__); \
		return RT_FALSE;                 \
	}

static void align(size_t *address)
{
	if (*address % INSIDE_FLS_BLOCK_SIZE != 0)
		*address = (*address / INSIDE_FLS_BLOCK_SIZE) * INSIDE_FLS_BLOCK_SIZE + INSIDE_FLS_BLOCK_SIZE;
}

rt_bool_t update_img_header_check(const T_BOOTER *img_param)
{
	// custom impl of tls_fwup_img_header_check() - it not up to date
	if (img_param->magic_no != SIGNATURE_WORD)
		RETURN("magic mismatch");
	if (img_param->img_type != IMG_TYPE_OLD_PLAIN)
		RETURN("unsupported img_type (%d)", img_param->img_type);
	if (img_param->zip_type != ZIP_FILE)
		RETURN("uncompressed image is not supported");

	psCrcContext_t crcContext;
	tls_crypto_crc_init(&crcContext, 0xFFFFFFFF, CRYPTO_CRC_TYPE_32, 3);
	for (uint i = 0; i < (sizeof(T_BOOTER) - 4) / 4; i++)
	{
		int value = *(((int *)img_param) + i);
		tls_crypto_crc_update(&crcContext, (unsigned char *)&value, 4);
	}
	unsigned int value = 0;
	tls_crypto_crc_final(&crcContext, &value);

	if (img_param->hd_checksum != value)
		RETURN("checksum mismatch: want: 0x%X, got: 0x%X", value, img_param->hd_checksum);

	size_t runaddr = img_param->run_img_addr | FLASH_BASE_ADDR;
	if (runaddr != CODE_RUN_START_ADDR)
		RETURN("code start address invalid: 0x%X != 0x%X", runaddr, CODE_RUN_START_ADDR);
	// if (runaddr % INSIDE_FLS_PAGE_SIZE)
	// 	LOG_E("runaddr(0x%X) not page aligned by %d", runaddr, INSIDE_FLS_PAGE_SIZE);

	size_t code_end = img_param->run_img_len + runaddr;
	align(&code_end);
	if (code_end > USER_ADDR_START)
		RETURN("code over user params [TOO_LARGE]: 0x%X > 0x%X", code_end, USER_ADDR_START);

	size_t updateaddr = img_param->upd_img_addr | FLASH_BASE_ADDR;
	if (updateaddr != CODE_UPD_START_ADDR)
		RETURN("update start address invalid: 0x%X != 0x%X", updateaddr, CODE_UPD_START_ADDR);

	size_t update_end = img_param->upd_img_len + updateaddr;
	align(&update_end);
	if (update_end > USER_ADDR_START)
		RETURN("update over user params [TOO_LARGE]: 0x%X > 0x%X", update_end, USER_ADDR_START);

	return RT_TRUE;
}
