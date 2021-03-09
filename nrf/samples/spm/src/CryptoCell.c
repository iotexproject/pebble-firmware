#include <zephyr.h>
#include <errno.h>
#include <cortex_m/tz.h>
#include <power/reboot.h>
#include <sys/util.h>
#include <autoconf.h>
#include <string.h>
#include <bl_validation.h>

#include "nrf_cc3xx_platform.h"
#include "nrf_cc3xx_platform_kmu.h"
#include "mbedtls/cc3xx_kmu.h"
#include "mbedtls/aes.h"
#include "mbedtls/ctr_drbg.h"
#include <mbedtls/ecdsa.h>
//#include <mbedtls/ctr_drbg.h>
#include <drivers/entropy.h>

#define   AES_KMU_SLOT			  2

#define ECPARAMS    MBEDTLS_ECP_DP_SECP256R1

#if !defined(ECPARAMS)
#define ECPARAMS    mbedtls_ecp_curve_list()->grp_id
#endif

#define TV_NAME(name) name " -- [" __FILE__ ":" STRINGIFY(__LINE__) "]"

typedef struct {
	const uint32_t src_line_num; /**< Test vector source file line number. */
	const uint32_t curve_type; /**< Curve type for test vector. */
	const int expected_sign_err_code; /**< Expected error code from ECDSA sign
									   operation. */
	const int expected_verify_err_code; /**< Expected result of following ECDSA
										 verify operation. */
	const char *p_test_vector_name; /**< Pointer to ECDSA test vector name. */
	char * p_input; /**< Pointer to ECDSA hash input in hex string format. */
	const char *
		p_qx; /**< Pointer to ECDSA public key X component in hex string
					   format. */
	const char *
		p_qy; /**< Pointer to ECDSA public key Y component in hex string
					   format. */
	const char *
		p_x; /**< Pointer to ECDSA private key component in hex string format. */
} test_vector_ecdsa_sign_t;


//mbedtls_ctr_drbg_context ctr_drbg_ctx;

test_vector_ecdsa_sign_t test_case_ecdsa_data = {
	.curve_type = MBEDTLS_ECP_DP_SECP256R1,
	.expected_sign_err_code = 0,
	.expected_verify_err_code = 0,
	.p_test_vector_name = TV_NAME("secp256r1 valid SHA256 1")
};

static test_vector_ecdsa_sign_t *p_test_vector_sign;

static char* m_ecdsa_input_buf;

static size_t hash_len, initOKFlg=0;

static int entropy_func(void *ctx, unsigned char *buf, size_t len)
{
	return entropy_get_entropy(ctx, buf, len);
}


#if defined(MBEDTLS_CTR_DRBG_C)
mbedtls_ctr_drbg_context ctr_drbg_ctx;
int (*drbg_random)(void *, unsigned char *, size_t) = &mbedtls_ctr_drbg_random;

static int init_drbg(const unsigned char *p_optional_seed, size_t len)
{
	static const unsigned char ncs_seed[] = "ncs_drbg_seed";

	const unsigned char *p_seed;

	if (p_optional_seed == NULL) {
		p_seed = ncs_seed;
		len = sizeof(ncs_seed);
	} else {
		p_seed = p_optional_seed;
	}

	const struct device *p_device =
	    device_get_binding(DT_LABEL(DT_CHOSEN(zephyr_entropy)));

	if (p_device == NULL)
		return -ENODEV;

	// Ensure previously run test is properly deallocated
	// (This frees the mutex inside ctr_drbg context)
	mbedtls_ctr_drbg_free(&ctr_drbg_ctx);
	mbedtls_ctr_drbg_init(&ctr_drbg_ctx);
	return mbedtls_ctr_drbg_seed(&ctr_drbg_ctx, entropy_func, (void *)p_device,
				     p_seed, len);
}
#elif defined(MBEDTLS_HMAC_DRBG_C)
mbedtls_hmac_drbg_context drbg_ctx;
static int (*drbg_random)(void *, unsigned char *, size_t) = &mbedtls_hmac_drbg_random;

int init_drbg(const unsigned char *p_optional_seed, size_t len)
{
	static const unsigned char ncs_seed[] = "ncs_drbg_seed";

	const unsigned char *p_seed;

	if (p_optional_seed == NULL) {
		p_seed = ncs_seed;
		len = sizeof(ncs_seed);
	} else {
		p_seed = p_optional_seed;
	}

	// Ensure previously run test is properly deallocated
	// (This frees the mutex inside hmac_drbg context)
	mbedtls_hmac_drbg_free(&drbg_ctx);
	mbedtls_hmac_drbg_init(&drbg_ctx);

	const struct device *p_device =
	    device_get_binding(DT_LABEL(DT_CHOSEN(zephyr_entropy)));

	if (!p_device)
		return -ENODEV;

	const mbedtls_md_info_t *p_info =
		mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);

	return mbedtls_hmac_drbg_seed(&drbg_ctx, p_info, entropy_func,
		(void *)p_device, p_seed, len);
}
#endif
#if 0
static int init_ctr_drbg(const unsigned char *p_optional_seed, size_t len)
{
	static const unsigned char ncs_seed[] = "ncs_drbg_seed";

	const unsigned char *p_seed;

	if (p_optional_seed == NULL) {
		p_seed = ncs_seed;
		len = sizeof(ncs_seed);
	} else {
		p_seed = p_optional_seed;
	}

        // v1.3.0-sdk-update
	//struct device *p_device = device_get_binding(CONFIG_ENTROPY_NAME);
    struct device *p_device =device_get_binding(DT_LABEL(DT_CHOSEN(zephyr_entropy)));
	if (p_device == NULL) {
		return -ENODEV;
	}
	mbedtls_ctr_drbg_free(&drbg_ctx);
	mbedtls_ctr_drbg_init(&ctr_drbg_ctx);
        ctr_drbg_ctx.entropy_len = 144;
		
	return mbedtls_ctr_drbg_seed(&ctr_drbg_ctx, entropy_func, p_device,
				     p_seed, len);
}
#endif
static void exec_test_case_ecdsa_sign(char *buf,int *len)
{
	int err_code = -1;
	unsigned int len_r,len_s;
	
	/* Prepare signer context. */
	mbedtls_ecdsa_context ctx_sign;
	mbedtls_ecdsa_init(&ctx_sign);

	err_code = mbedtls_ecp_group_load(&ctx_sign.grp,
					  p_test_vector_sign->curve_type);

	/* Get private key. */
	err_code = mbedtls_mpi_read_string(&ctx_sign.d, 16,
					   p_test_vector_sign->p_x);

	/* Verify keys. */
	err_code = mbedtls_ecp_check_privkey(&ctx_sign.grp, &ctx_sign.d);
	/* Prepare and generate the ECDSA signature. */
	/* Note: The contexts do not contain these (as is the case for e.g. Q), so simply share them here. */
	mbedtls_mpi r;
	mbedtls_mpi s;
	mbedtls_mpi_init(&r);
	mbedtls_mpi_init(&s);

	//start_time_measurement();

	err_code = mbedtls_ecdsa_sign(&ctx_sign.grp, &r, &s, &ctx_sign.d,
				      m_ecdsa_input_buf, hash_len,
				      drbg_random, &ctr_drbg_ctx);    
	//stop_time_measurement();
    len_r = mbedtls_mpi_size(&r);
    len_s  = mbedtls_mpi_size(&s);
    mbedtls_mpi_write_binary(&r, buf, len_r);
    mbedtls_mpi_write_binary(&s, buf+len_r, len_s);
	*len = len_r+len_s;

	/* Free resources. */
	mbedtls_mpi_free(&r);
	mbedtls_mpi_free(&s);
	mbedtls_ecdsa_free(&ctx_sign);
}
__TZ_NONSECURE_ENTRY_FUNC
int initECDSA_sep256r(void)
{	
	if(Initcc3xx())
		return -1;		
    if (init_drbg(NULL, 0) != 0) {
		initOKFlg = 0;        	
		return  1;
    }   		
	initOKFlg = 1;
	return 0;
}
__TZ_NONSECURE_ENTRY_FUNC
int doESDA_sep256r_Sign(char *inbuf, uint32_t len, char *buf, int* sinlen)
{
	char shaBuf[32];
	int err_code = -1, i;
	if(!initOKFlg)
		return -1;	

	p_test_vector_sign = &test_case_ecdsa_data;

	err_code = mbedtls_sha256_ret(inbuf, len,  shaBuf,false);
	
	p_test_vector_sign->p_input = shaBuf;
	m_ecdsa_input_buf = shaBuf;
	hash_len = 32;

	exec_test_case_ecdsa_sign(buf, sinlen);

	return err_code;
}
void hex2str(char* buf_hex, int len, char *str)
{
	int i,j;
	for(i =0,j=0; i< len; i++)
	{
		str[j++] = (buf_hex[i]&0x0F) > 9 ? ((buf_hex[i]&0x0F)-10 +'A'):((buf_hex[i]&0x0F)+'0');
		str[j++] = (buf_hex[i]&0xF0) > 0x90 ? (((buf_hex[i]&0xF0)>>4)-10 + 'A'):(((buf_hex[i]&0xF0)>>4)+'0');		
	}
	str[j] = 0;	
}

__TZ_NONSECURE_ENTRY_FUNC
int GenRandom(char *out)
{	
	char buf[8];
	drbg_random(&ctr_drbg_ctx, buf, sizeof(buf));
	hex2str(buf, sizeof(buf),out);
	return  0;
}

__TZ_NONSECURE_ENTRY_FUNC
int iotex_random(void)
{
	union
	{
		int  dat;
		char buf[4];
	}random32;
	
	drbg_random(&ctr_drbg_ctx, random32.buf, sizeof(random32.buf));
	return random32.dat;
}
__TZ_NONSECURE_ENTRY_FUNC
int gen_ecc_key(char *buf, int  len, char* buf_p, int len_p)
{	
	int ret = 0, strLen;
	mbedtls_ecdsa_context ctx_sign;
	mbedtls_ecdsa_init( &ctx_sign );	

    if(( ret = mbedtls_ecdsa_genkey(&ctx_sign, ECPARAMS,drbg_random, &ctr_drbg_ctx)) != 0)
    {        	  		
		return ret;
    }	
	
	// get private key in the format of  hex-string
	if((ret = mbedtls_mpi_write_string(&ctx_sign.d, 16, buf, len, &strLen) != 0))
	{		
		return ret;		
	}
	if((ret = mbedtls_ecp_point_write_binary( &ctx_sign.grp, &ctx_sign.Q, MBEDTLS_ECP_PF_UNCOMPRESSED, &strLen, buf_p, len_p )) != 0)
	{		
		return ret;		
	}
	return ret;
}

__TZ_NONSECURE_ENTRY_FUNC
void SetEccPrivKey(uint8_t *key)
{
    static uint8_t decrypted_buf[66];
    memcpy(decrypted_buf, key, 64);
    decrypted_buf[64] = 0;
    test_case_ecdsa_data.p_x = (const char *)decrypted_buf;
}
__TZ_NONSECURE_ENTRY_FUNC
void genAESKey(uint8_t *key, int len)
{
    drbg_random(&ctr_drbg_ctx, key, sizeof(key));
}