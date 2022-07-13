#include "endian_conv.h"

/* Endian convert */
int endian_is_bigendian() {
    static uint16_t test = 0x1234;
    static const uint8_t *c = (const uint8_t *)&test;
    return c[0] == 0x12 && c[1] == 0x34;
}

uint16_t endian_swap16(uint16_t value) {
    return (value >> 8) | (value << 8);
}

uint32_t endian_swap32(uint32_t value) {
    uint32_t temp = ((value << 8) & 0xFF00FF00) | ((value >> 8) & 0xFF00FF);
    return (temp << 16) | (temp >> 16);
}

uint64_t endian_swap64(uint64_t value) {
    value = ((value & 0x00000000FFFFFFFFull) << 32) | ((value & 0xFFFFFFFF00000000ull) >> 32);
    value = ((value & 0x0000FFFF0000FFFFull) << 16) | ((value & 0xFFFF0000FFFF0000ull) >> 16);
    value = ((value & 0x00FF00FF00FF00FFull) << 8)  | ((value & 0xFF00FF00FF00FF00ull) >> 8);
    return value;
}

