/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */
#include <zephyr.h>
#include <errno.h>
#include <cortex_m/tz.h>
#include <power/reboot.h>
#include <sys/util.h>
#include <autoconf.h>
#include <string.h>
#include <bl_validation.h>

#include <sys/printk.h>
#include "nrf_cc3xx_platform.h"
#include "nrf_cc3xx_platform_kmu.h"
#include "mbedtls/cc3xx_kmu.h"
#include "mbedtls/aes.h"
#include "mbedtls/ctr_drbg.h"


#if USE_PARTITION_MANAGER
#include <pm_config.h>
#endif
#ifdef CONFIG_SPM_SERVICE_FIND_FIRMWARE_INFO
#include <fw_info.h>
#endif

/*
 * Secure Entry functions to allow access to secure services from non-secure
 * firmware.
 *
 * Note: the function will be located in a Non-Secure
 * Callable region of the Secure Firmware Image.
 *
 * These should not be called directly. Instead call them through their wrapper
 * functions, e.g. call spm_request_read_nse() via spm_request_read().
 */

#ifdef CONFIG_SPM_SERVICE_RNG
#ifdef MBEDTLS_CONFIG_FILE
#include MBEDTLS_CONFIG_FILE
#else
#include "mbedtls/config.h"
#endif /* MBEDTLS_CONFIG_FILE */

#include <mbedtls/platform.h>
#include <mbedtls/entropy_poll.h>
#endif /* CONFIG_SPM_SERVICE_RNG */


int spm_secure_services_init(void)
{
	int err = 0;

#ifdef CONFIG_SPM_SERVICE_RNG
	mbedtls_platform_context platform_ctx = {0};
	err = mbedtls_platform_setup(&platform_ctx);
#endif
	return err;
}

#ifdef CONFIG_SPM_SERVICE_READ

#define FICR_BASE               NRF_FICR_S_BASE
#define FICR_PUBLIC_ADDR        (FICR_BASE + 0x204)
#define FICR_PUBLIC_SIZE        0xA1C
#define FICR_RESTRICTED_ADDR    (FICR_BASE + 0x130)
#define FICR_RESTRICTED_SIZE    0x8

struct read_range {
	uint32_t start;
	size_t size;
};


__TZ_NONSECURE_ENTRY_FUNC
int spm_request_read_nse(void *destination, uint32_t addr, size_t len)
{
	static const struct read_range ranges[] = {
#ifdef PM_MCUBOOT_ADDRESS
		/* Allow reads of mcuboot metadata */
		{.start = PM_MCUBOOT_PAD_ADDRESS,
		 .size = PM_MCUBOOT_PAD_SIZE},
#endif
		{.start = FICR_PUBLIC_ADDR,
		 .size = FICR_PUBLIC_SIZE},
		{.start = FICR_RESTRICTED_ADDR,
		 .size = FICR_RESTRICTED_SIZE},
	};

	if (destination == NULL || len <= 0) {
		return -EINVAL;
	}

	for (size_t i = 0; i < ARRAY_SIZE(ranges); i++) {
		uint32_t start = ranges[i].start;
		uint32_t size = ranges[i].size;

		if (addr >= start && addr + len <= start + size) {
			memcpy(destination, (const void *)addr, len);
			return 0;
		}
	}

	return -EPERM;
}
#endif /* CONFIG_SPM_SERVICE_READ */


#ifdef CONFIG_SPM_SERVICE_REBOOT
__TZ_NONSECURE_ENTRY_FUNC
void spm_request_system_reboot_nse(void)
{
	sys_reboot(SYS_REBOOT_COLD);
}
#endif /* CONFIG_SPM_SERVICE_REBOOT */


#ifdef CONFIG_SPM_SERVICE_RNG
__TZ_NONSECURE_ENTRY_FUNC
int spm_request_random_number_nse(uint8_t *output, size_t len, size_t *olen)
{
	int err;

	if (len != MBEDTLS_ENTROPY_MAX_GATHER) {
		return -EINVAL;
	}

	err = mbedtls_hardware_poll(NULL, output, len, olen);
	return err;
}
#endif /* CONFIG_SPM_SERVICE_RNG */


#ifdef CONFIG_SPM_SERVICE_FIND_FIRMWARE_INFO
__TZ_NONSECURE_ENTRY_FUNC
int spm_firmware_info_nse(uint32_t fw_address, struct fw_info *info)
{
	const struct fw_info *tmp_info;

	if (info == NULL) {
		return -EINVAL;
	}

	tmp_info = fw_info_find(fw_address);

	if (tmp_info != NULL) {
		memcpy(info, tmp_info, sizeof(*tmp_info));
		return 0;
	}

	return -EFAULT;
}
#endif /* CONFIG_SPM_SERVICE_FIND_FIRMWARE_INFO */


#ifdef CONFIG_SPM_SERVICE_PREVALIDATE
__TZ_NONSECURE_ENTRY_FUNC
int spm_prevalidate_b1_upgrade_nse(uint32_t dst_addr, uint32_t src_addr)
{
	if (!bl_validate_firmware_available()) {
		return -ENOTSUP;
	}
	bool result = bl_validate_firmware(dst_addr, src_addr);
	return result;
}
#endif /* CONFIG_SPM_SERVICE_PREVALIDATE */


#ifdef CONFIG_SPM_SERVICE_BUSY_WAIT
__TZ_NONSECURE_ENTRY_FUNC
void spm_busy_wait_nse(uint32_t busy_wait_us)
{
	k_busy_wait(busy_wait_us);
}
#endif /* CONFIG_SPM_SERVICE_BUSY_WAIT */

__TZ_NONSECURE_ENTRY_FUNC
int store_key_in_kmu(uint32_t slot_id, char *key, char *read)
{
	int ret;
	if(slot_id >=2) {
		ret = nrf_cc3xx_platform_kmu_write_key_slot(
			slot_id,
			NRF_CC3XX_PLATFORM_KMU_AES_ADDR,
			NRF_CC3XX_PLATFORM_KMU_DEFAULT_PERMISSIONS,
			key);
		if((ret != NRF_CC3XX_PLATFORM_SUCCESS) && (ret != NRF_CC3XX_PLATFORM_ERROR_KMU_ALREADY_FILLED))
		{
			//printk("Could not write KMU slot %i. Try erasing the board...\n", slot_id);
			return ret;
		}
	}
	else
	{
		/*
		*(int *)(0x50039000 + 0x500) = slot_id + 1;
		ret = *(int *)(0x00FF8800+ slot_id * 0x10+ 0 * 0x4);
		read[0] = ret&0x000000FF;
		read[1] = (ret&0x0000FF00)>>8;
		read[2] = (ret&0x00FF0000)>>16;
		read[3] = (ret&0xFF000000)>>24;

		*(int *)(0x50039000 + 0x500) = 0;
		*/
		ret = nrf_cc3xx_platform_kmu_write_kdr_slot(key);
		if((ret != NRF_CC3XX_PLATFORM_SUCCESS) && (ret != NRF_CC3XX_PLATFORM_ERROR_KMU_ALREADY_FILLED))
		{
			//printk("Could not write KMU slot %i. Try erasing the board...\n", slot_id);
			return ret;
		}
		ret = nrf_cc3xx_platform_kmu_push_kdr_slot_and_lock();
		if(ret != NRF_CC3XX_PLATFORM_SUCCESS)
			return ret;
	}
	return 0;
}
__TZ_NONSECURE_ENTRY_FUNC
int Initcc3xx(void)
{
	if (nrf_cc3xx_platform_init() != 0)
	{
		//printk("Failed to initialize CC3xx platform.\n");
		return -1;
	}
	return 0;	
}
__TZ_NONSECURE_ENTRY_FUNC
int cc3xx_encrypt(uint32_t slot_id, char *plain_text,  char *cipher_text)
{
	int ret;
	mbedtls_aes_context ctx = {0};
	mbedtls_aes_init(&ctx);	
	char *label = "Sealing Key";
	char *contex = "";	
	// Set to use direct shadow key for encryption.
	if(slot_id >=2) {
		ret = mbedtls_aes_setkey_enc_shadow_key(&ctx, slot_id, 128);
	}	
	else{
		ret = mbedtls_aes_setkey_enc_shadow_key_derived(&ctx, slot_id, 128, label, strlen(label), contex, strlen(contex));
	}	
	if (ret != 0)
	{		
		return ret;
	}	
	
	mbedtls_aes_encrypt(&ctx, plain_text, cipher_text);	
	return 0;
}

int  cc3xx_decrypt(uint32_t slot_id, char *plain_text_decrypted,  char *cipher_text)
{
    int ret;
	char *label = "Sealing Key";
	char *contex = "";

	mbedtls_aes_context ctx = {0};
	mbedtls_aes_init(&ctx);
	// Set to use direct shadow key for decryption.
	if(slot_id >=2) {	
		ret = mbedtls_aes_setkey_dec_shadow_key(&ctx, slot_id, 128);
	}
	else {
		ret = mbedtls_aes_setkey_dec_shadow_key_derived(&ctx, slot_id, 128, label, strlen(label), contex, strlen(contex));
	}		
	if (ret != 0)
	{		
		return ret;
	}

	mbedtls_aes_decrypt(&ctx, cipher_text, plain_text_decrypted);
	return 0;	
}