#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "abi_pack.h"
#include "endian_conv.h"

/*
 * it packs calldata only for the specific func: "publish(bytes)"
 * cannot be used to pack general functions
 *
 * caller needs to free the returned pointer
 */
uint8_t *abi_pack_publish(const uint8_t *input, uint64_t size, uint64_t *out_size) {
    uint8_t *data, *curr, *ptrSize;
    uint64_t alignSize, bigSize;

    // align size to 32-byte
    alignSize = ((size + 31) >> 5) << 5;

    if ((data = (uint8_t *)malloc(
                    4 +  // len of function signature
                    32 + // offset to start of input
                    32 + // len of input
                    alignSize // actual input
                )) == NULL) {
        *out_size = 0;
        return data;
    };

    // clear first 68 bytes
    memset(data, 0, 68);

    // pack 4-byte func signature
    // { "7fd28346": "publish(bytes)" }
    data[0] = 0x7f;

    data[1] = 0xd2;

    data[2] = 0x83;

    data[3] = 0x46;

    // offset to start of input is 32-bytes
    curr = data + 4 + 31;

    *curr++ = 32;

    // len of input encoded in big-endian
    bigSize = endian_is_bigendian() ? size : endian_swap64(size);

    ptrSize = (uint8_t *)&bigSize;

    curr += 24;

    for (int i = 0; i < 8; i++) {
        *curr++ = *ptrSize++;
    };

    // copy input data
    for (int i = 0; i < size; i++) {
        *curr++ = *input++;
    };

    // pad 0
    for (int i = 0; i < alignSize - size; i++) {
        *curr++ = 0;
    };

    *out_size = 68 + alignSize;

    return data;
}
