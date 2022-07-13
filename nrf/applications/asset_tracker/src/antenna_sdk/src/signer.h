#ifndef _IOTEX_SIGNER_H_
#define _IOTEX_SIGNER_H_


#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>


#define SIG_HASH_SIZE 32
#define SIG_PRIVATE_KEY_SIZE 32
#define SIG_PUBLIC_KEY_SIZE 65
#define SIG_SIGNATURE_SIZE 65

int signer_str2hex(const char *str, uint8_t *hex, size_t size);
int signer_hex2str(const uint8_t *hex, size_t hex_size, char *str, size_t str_size);

void signer_get_hash(const uint8_t *data, size_t size, uint8_t hash[SIG_HASH_SIZE]);
void signer_get_public_key(const uint8_t private_key[SIG_PRIVATE_KEY_SIZE], uint8_t public_key[SIG_PUBLIC_KEY_SIZE]);
int signer_get_signature(const uint8_t *data, size_t size, const uint8_t private_key[SIG_PRIVATE_KEY_SIZE], uint8_t signature[SIG_SIGNATURE_SIZE]);


#ifdef	__cplusplus
}
#endif

#endif /*_IOTEX_SIGNER_H_ */
