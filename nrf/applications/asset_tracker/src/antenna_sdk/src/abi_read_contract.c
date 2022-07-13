#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "abi_read_contract.h"
#include "endian_conv.h"

// input is the bytes returned by reading contract's getDeviceOrderByID()
// parse the bytes and return start height
// returns 0 for error
uint64_t abi_get_order_start(const char *input, size_t size) {
    if (size < 32) {
        return 0;
    }

    uint64_t start = *(uint64_t *)(input + 24);
    if (!endian_is_bigendian()) {
        start = endian_swap64(start);
    }
    return start;
}

// input is the bytes returned by reading contract's getDeviceOrderByID()
// parse the bytes and return duration
// returns 0 for error
uint32_t abi_get_order_duration(const char *input, size_t size) {
    if (size < 64) {
        return 0;
    }

    uint32_t duration = *(uint32_t *)(input + 60);
    if (!endian_is_bigendian()) {
        duration = endian_swap32(duration);
    }
    return duration;
}

// input is the bytes returned by reading contract's getDeviceOrderByID()
// parse the bytes and return subscriber's storage endpoint
// returns NULL for error
const char *abi_get_order_endpoint(const char *input, size_t size) {
    if (size < 96) {
        return NULL;
    }

    // bytes 64-96 encodes the offset
    uint16_t offset = *(uint16_t *)(input + 94);
    if (!endian_is_bigendian()) {
        offset = endian_swap16(offset);
    }

    if (size < offset + 32) {
        return NULL;
    }
    input += offset;

    // next 32 bytes encodes the length
    // assume endpoint is less than 65535 bytes
    uint16_t length = *(uint16_t *)(input + 30);
    if (!endian_is_bigendian()) {
        length = endian_swap16(length);
    }

    if (size < offset + 32 + length) {
        return NULL;
    }

    // read 'length' bytes
    char *endpoint;
    if ((endpoint = (char *)malloc(length+1)) == NULL) {
        return NULL;
    }
    memcpy(endpoint, input + 32, length);
    endpoint[length] = 0;

    return endpoint;
}

// input is the bytes returned by reading contract's getDeviceOrderByID()
// parse the bytes and return subscriber's storage access token
// returns NULL for error
const char *abi_get_order_token(const char *input, size_t size) {
    if (size < 128) {
        return NULL;
    }

    // bytes 96-128 encodes the offset
    uint16_t offset = *(uint16_t *)(input + 126);
    if (!endian_is_bigendian()) {
        offset = endian_swap16(offset);
    }

    if (size < offset + 32) {
        return NULL;
    }
    input += offset;

    // next 32 bytes encodes the length
    // assume token is less than 65535 bytes
    uint16_t length = *(uint16_t *)(input + 30);
    if (!endian_is_bigendian()) {
        length = endian_swap16(length);
    }

    if (size < offset + 32 + length) {
        return NULL;
    }

    // read 'length' bytes
    char *token;
    if ((token = (char *)malloc(length+1)) == NULL) {
        return NULL;
    }
    memcpy(token, input + 32, length);
    token[length] = 0;

    return token;
}
