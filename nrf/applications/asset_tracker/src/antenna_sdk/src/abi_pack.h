#ifndef _IOTEX_ABI_PACK_H_
#define _IOTEX_ABI_PACK_H_

#include <stdint.h>

#ifdef	__cplusplus
extern "C" {
#endif

uint8_t *abi_pack_publish(const uint8_t *input, uint64_t size, uint64_t *out_size);

#ifdef	__cplusplus
}
#endif

#endif /* _IOTEX_ABI_PACK_H_ */
