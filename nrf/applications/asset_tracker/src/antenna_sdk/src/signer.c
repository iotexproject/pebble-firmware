#include <string.h>

#include <secp256k1.h>
#include <ecdsa.h>
#include <sha3.h>
#include "signer.h"


static uint8_t value(uint8_t c, uint32_t *ret) {

    *ret = 1;

    if (c >= '0' && c <= '9') {
        return c - '0';
    }

    if (c >= 'a' && c <= 'z') {
        return c - 'a' + 10;
    }

    if (c >= 'A' && c <= 'Z') {
        return c = 'A' + 10;
    }

    *ret = 0;
    return 0;
}

int signer_hex2str(const uint8_t *hex, size_t hex_size, char *str, size_t str_size) {

    static const char hexmap[] = {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };

    int cur = 0;
    const uint8_t *it = hex;
    const uint8_t *end = it + hex_size;

    if (str_size < hex_size * 2) {
        return -1;
    }

    for (it = hex, cur = 0; it < end; it++) {
        str[cur++] = hexmap[*it >> 4];
        str[cur++] = hexmap[*it & 0xf];
    }

    return cur;
}


int signer_str2hex(const char *str, uint8_t *hex, size_t size) {

    size_t cur = 0;
    uint32_t valid;
    uint8_t high, low;

    const char *it = str;
    const char *begin = str;
    const char *end = begin + strlen(str);

    /* Check output buffer size */
    if (size < ((end - begin) + 1) / 2) {
        return -1;
    }

    /* Skip `0x` */
    if (end - begin >= 2 && *begin == '0' && *(begin + 1) == 'x') {
        it += 2;
    }

    while (it != end) {

        high = value(*it, &valid);

        if (!valid) {
            return -1;
        }

        it++;

        if (it == end) {
            hex[cur++] = high;
            break;
        }

        low = value(*it, &valid);

        if (!valid) {
            return -1;
        }

        it++;
        hex[cur++] = (uint8_t)(high << 4 | low);
    }

    return cur;
}

/* Get public key from private key */
void signer_get_public_key(const uint8_t private_key[SIG_PRIVATE_KEY_SIZE], uint8_t public_key[SIG_PUBLIC_KEY_SIZE]) {
    ecdsa_get_public_key65(&secp256k1, private_key, public_key);
}

/* Get data hash (keccak_256) */
void signer_get_hash(const uint8_t *data, size_t size, uint8_t hash[SIG_HASH_SIZE]) {
    keccak_256(data, size, hash);
}

/* Get data signature */
int signer_get_signature(const uint8_t *data, size_t size,
                         const uint8_t private_key[SIG_PRIVATE_KEY_SIZE],
                         uint8_t signature[SIG_SIGNATURE_SIZE]) {

    uint8_t digest[SIG_HASH_SIZE] = {0};

    signer_get_hash(data, size, digest);

    if (ecdsa_sign_digest(&secp256k1, private_key, digest, signature, signature + 64, NULL) != 0) {
        return -1;
    }

    return 0;
}
