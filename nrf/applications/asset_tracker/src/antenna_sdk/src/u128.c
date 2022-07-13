#include "u128.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <limits.h>
#include <inttypes.h>


/* Compare uint128_t is equal, equal return 1 */
int u128_equal(uint128_t a, uint128_t b) {
#ifdef _NO_128INT_
    return memcmp(a.raw, b.raw, sizeof(a.raw)) == 0;
#else
    return a == b;
#endif
}

int u128_is_less(uint128_t a, uint128_t b) {
#ifdef _NO_128INT_
    size_t a_len = strlen(a.raw);
    size_t b_len = strlen(b.raw);

    if (a_len < b_len) {
        return 1;
    }
    else if (b_len < a_len) {
        return 0;
    }
    else {
        return memcmp(a.raw, b.raw, a_len) < 0;
    }

#else
    return a < b;
#endif
}

/* Print uint128_t */
void u128_print(uint128_t u128) {

    char u128_str[UINT128_RAW_MAX_LEN];
    fprintf(stdout, "%s\n", u1282str(u128, u128_str, sizeof(u128_str)));
}


/* Construct uint128_t from string */
uint128_t construct_u128(const char *str) {

    uint128_t u128;
    str2u128(str, &u128);
    return u128;
}


/* Convert uint128_t to string */
void str2u128(const char *str, uint128_t *num) {

    assert(str != NULL);
    assert(num != NULL);

#ifdef _NO_128INT_
    char *dest = NULL;
    const char *src = NULL;
    size_t str_len = strlen(str);
    size_t raw_size = sizeof(num->raw);
    memset(num->raw, 0, sizeof(num->raw));

    /* String too long */
    if (str_len > raw_size - 1) {
        num->raw[0] = '0';
        return;
    }

    /* Check and copy string to uint128_t.raw */
    for (src = str, dest = num->raw; *src; src++, dest++) {

        if (*src >= '0' && *src <= '9') {
            *dest = *src;
        }
        else {
            memset(num->raw, 0, sizeof(num->raw));
            num->raw[0] = '0';
            return;
        }
    }

#else
    uint128_t u128 = 0;
    const char *c = NULL;

    for (c = str, u128 = 0; *c; c++) {
        if (*c >= '0' && *c <= '9') {
            u128 = u128 * 10 + (*c - '0');
        }
        else {
            u128 = 0;
            break;
        }
    }

    *num = u128;
#endif
}

/*
 * @brief: convert uint128_t to string end with zero
 * #num: number to convert
 * #str: string buffer to save number
 * #str_max_len: #str buffer max length
 * $return: successed return string buffer (#str), failed return NULL(#str buf too short)
 */
char *u1282str(uint128_t num, char *str, size_t str_max_len) {

    assert(str != NULL);
    assert(str_max_len >= 2);

#ifdef _NO_128INT_
    size_t raw_len = strlen(num.raw);

    if (str_max_len < raw_len + 1) {
        return NULL;
    }

    memcpy(str, num.raw, raw_len);
    str[raw_len] = 0;
#else
    char temp;
    int remaining = 0;
    char *start = str, *end = str;

    if (num == 0) {
        str[0] = '0';
        str[1] = 0;
        return str;
    }

    while (num != 0) {
        if (end - start < str_max_len - 1) {
            remaining = num % 10;
            *end = remaining + '0';
            num /= 10;
            end++;
        }
        else {
            return NULL;
        }
    }

    *end = 0;

    /* Reverse end and start */
    while (start < --end) {
        temp = *start;
        *start = *end;
        *end = temp;
        start++;
    }

#endif

    return str;
}

