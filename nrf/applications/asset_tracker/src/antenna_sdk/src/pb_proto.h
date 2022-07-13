#ifndef _IOTEX_PB_PROTO_H_
#define _IOTEX_PB_PROTO_H_

#ifdef	__cplusplus
extern "C" {
#endif

#include "pb_pack.h"

typedef enum {
    TX_AMOUNT = 1,
    TX_RECIPIENT,
    TX_PAYLOAD
} proto_em_transfer;

typedef enum {
    EX_AMOUNT = 1,
    EX_CONTRACT,
    EX_DATA
} proto_em_execution;

typedef enum {
    AC_VERSION = 1,
    AC_NONCE,
    AC_GAS_LIMIT,
    AC_GAS_PRICE,
} proto_em_action_core;

typedef enum {
    ACTION_CORE = 1,
    ACTION_SND_PUBKEY,
    ACTION_SIGNATURE
} proto_em_action;

typedef enum {
    ACT_TRANSFER = 10,
    ACT_EXECUTION = 12
} proto_em_action_id;


struct iotex_st_transfer;
struct iotex_st_execution;

int proto_gen_tx_action(const struct iotex_st_transfer *tx, uint8_t *action_bytes, size_t max_size);
int proto_gen_ex_action(const struct iotex_st_execution *ex, uint8_t *action_bytes, size_t max_size);

#ifdef	__cplusplus
}
#endif


#endif /* _IOTEX_PB_PROTO_H_ */
