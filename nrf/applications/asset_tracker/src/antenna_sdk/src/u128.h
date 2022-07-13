#ifndef _IOTEX_EMB_U128_H_
#define _IOTEX_EMB_U128_H_

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>

#define UINT128_RAW_MAX_LEN 40
#define _NO_128INT_

#ifdef _NO_128INT_
typedef struct {
    char raw[UINT128_RAW_MAX_LEN];
} uint128_t;
#else
typedef __uint128_t uint128_t;
#endif

void u128_print(uint128_t u128);
int u128_equal(uint128_t a, uint128_t b);
int u128_is_less(uint128_t a, uint128_t b);

uint128_t construct_u128(const char *str);

void str2u128(const char *str, uint128_t *number);
char *u1282str(uint128_t num, char *str, size_t str_max_len);

#ifdef	__cplusplus
}
#endif

#endif /* _IOTEX_EMB_U128_H_ */
