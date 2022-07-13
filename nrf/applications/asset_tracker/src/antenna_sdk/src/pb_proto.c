#include <assert.h>
#include "signer.h"
#include "pb_proto.h"
#include "iotex_emb.h"

#define SAFE_STRLEN(s) ((s) ? strlen(s) : 0)
#define GET_ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

/*
 * @brief: generate action bytes
 * #core: action core params
 * #action_id: action id (proto_em_action_id)
 * #action: specific action messages
 * #action_size: #action item size
 * #action_bytes: store action bytes
 * #max_size: indicate #action_bytes max size in bytes
 * #private_key: account private key
 * $return: success retrun #action_bytes actual size, failed return negative error code
 */
static int proto_gen_action(const iotex_st_act_core *core,
                            uint8_t action_id, const pb_st_item *action, size_t action_size,
                            uint8_t *action_bytes, size_t action_bytes_max_size) {

    assert(core != NULL);
    assert(action != NULL);
    assert(action_bytes != NULL);
    assert(core->privateKey != NULL);

    int pb_len;
    uint8_t action_signature[SIG_SIGNATURE_SIZE] = {0};
    uint8_t public_key_bytes[SIG_PUBLIC_KEY_SIZE] = {0};
    uint8_t private_key_bytes[SIG_PRIVATE_KEY_SIZE] = {0};

    /* Private key string to bytes */
    if (signer_str2hex(core->privateKey, private_key_bytes, sizeof(private_key_bytes)) != SIG_PRIVATE_KEY_SIZE) {
        return -1;
    }

    /* Get public key from private key */
    signer_get_public_key(private_key_bytes, public_key_bytes);

    pb_st_item act_core[] = {
        {PB_WT_VARINT, AC_VERSION, (void *)core->version},
        {PB_WT_VARINT, AC_NONCE, (void *)core->nonce},
        {PB_WT_VARINT, AC_GAS_LIMIT, (void *)core->gasLimit},
        {PB_WT_LD, AC_GAS_PRICE, (void *)core->gasPrice, SAFE_STRLEN(core->gasPrice)},
        {PB_WT_EMB, action_id, (void *)action, action_size},
    };

    pb_st_item tx_action[] = {
        {PB_WT_EMB, ACTION_CORE, (void *)act_core, GET_ARRAY_SIZE(act_core)},
        {PB_WT_LD, ACTION_SND_PUBKEY, (void *)public_key_bytes, sizeof(public_key_bytes)},
        {PB_WT_LD, ACTION_SIGNATURE, (void *)action_signature, sizeof(action_signature)},
    };

    /* Pack act_core, calc hash, generate action signature */
    if ((pb_len = pb_pack(act_core, GET_ARRAY_SIZE(act_core), action_bytes, action_bytes_max_size)) < 0) {
        return pb_len;
    }

    if (signer_get_signature(action_bytes, pb_len, private_key_bytes, action_signature) != 0) {
        return -1;
    }

    return pb_pack(tx_action, GET_ARRAY_SIZE(tx_action), action_bytes, action_bytes_max_size);
}

int proto_gen_tx_action(const struct iotex_st_transfer *tx, uint8_t *action_bytes, size_t max_size) {

    pb_st_item tx_msg[] = {
        {PB_WT_LD, TX_AMOUNT, (void *)tx->amount, SAFE_STRLEN(tx->amount)},
        {PB_WT_LD, TX_RECIPIENT, (void *)tx->recipient, SAFE_STRLEN(tx->recipient)},
        {PB_WT_LD, TX_PAYLOAD, (void *)tx->payload, tx->payloadLength},
    };

    return proto_gen_action(&tx->core, ACT_TRANSFER, tx_msg, GET_ARRAY_SIZE(tx_msg), action_bytes, max_size);
}

int proto_gen_ex_action(const struct iotex_st_execution *ex, uint8_t *action_bytes, size_t max_size) {

    pb_st_item ex_msg[] = {
        {PB_WT_LD, EX_AMOUNT, (void *)ex->amount, SAFE_STRLEN(ex->amount)},
        {PB_WT_LD, EX_CONTRACT, (void *)ex->contract, SAFE_STRLEN(ex->contract)},
        {PB_WT_LD, EX_DATA, (void *)ex->data, ex->dataLength},
    };

    return proto_gen_action(&ex->core, ACT_EXECUTION, ex_msg, GET_ARRAY_SIZE(ex_msg), action_bytes, max_size);
}
