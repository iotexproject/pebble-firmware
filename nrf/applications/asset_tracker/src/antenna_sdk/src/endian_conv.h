#ifndef _IOTEX_ENDIAN_CONV_H_
#define _IOTEX_ENDIAN_CONV_H_

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>

int endian_is_bigendian();
uint16_t endian_swap16(uint16_t value);
uint32_t endian_swap32(uint32_t value);
uint64_t endian_swap64(uint64_t value);

#define ENDIAN_H2LE16(x) (endian_is_bigendian() ? endian_swap16(x) : (x))
#define ENDIAN_H2LE32(x) (endian_is_bigendian() ? endian_swap32(x) : (x))
#define ENDIAN_H2LE64(x) (endian_is_bigendian() ? endian_swap64(x) : (x))

#ifdef	__cplusplus
}
#endif


#endif /* _IOTEX_ENDIAN_CONV_ */

