#ifndef _IOTEX_EMB_CONFIG_H_
#define _IOTEX_EMB_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

struct iotex_st_config;

/* URL */
//#define IOTEX_EMB_BASE_URL "https://pharos.iotex.io/v%d/"
//#define IOTEX_EMB_BASE_URL "https://www.amazon.com/v%d/"
//#define   IOTEX_EMB_BASE_URL "https://iotex.io/v%d/"
//#define IOTEX_EMB_BASE_URL "http://122.51.248.171:9001/v%d/"
//#define IOTEX_EMB_BASE_URL "https://www.yahoo.com/v%d/"
//#define  IOTEX_EMB_BASE_URL  "http://www.dxzy163.com:80/v%d/"
#define     IOTEX_EMB_BASE_URL   "http://ec2-52-83-24-120.cn-northwest-1.compute.amazonaws.com.cn:8192/v%d/"
#define IOTEX_EMB_MAX_URL_LEN 256
#define IOTEX_EMB_MAX_ACB_LEN 1024

/* Response */
#define IOTEX_EBM_MAX_RES_LEN (3 * 1024)

#define  IS_PLATFORM_NRF9160   1
#define   NOT_PLATFORM_NRF9160  (!IS_PLATFORM_NRF9160)

void print_config();
struct iotex_st_config get_config();

void clear_config(void);
int init_config(const struct iotex_st_config *context);

#ifdef __cplusplus
}
#endif

#endif /* _IOTEX_EMB_CONFIG_H_ */
