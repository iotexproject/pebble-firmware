#ifndef _IOTEX_EMB_H_
#define _IOTEX_EMB_H_


#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "u128.h"


/* Data type length */
#define IOTEX_EMB_LIMIT_ACCOUNT_LEN 42
#define IOTEX_EMB_LIMIT_TIME_LEN 21
#define IOTEX_EMB_LIMIT_HASH_LEN 65
#define IOTEX_EMB_LIMIT_KEY_LEN 89
#define IOTEX_EMB_LIMIT_ID_LEN 16


/* Basic type typedef */
typedef char iotex_t_account[IOTEX_EMB_LIMIT_ACCOUNT_LEN];
typedef char iotex_t_hash[IOTEX_EMB_LIMIT_HASH_LEN];
typedef char iotex_t_time[IOTEX_EMB_LIMIT_TIME_LEN];
typedef char iotex_t_key[IOTEX_EMB_LIMIT_KEY_LEN];
typedef char iotex_t_id[IOTEX_EMB_LIMIT_ID_LEN];
typedef uint128_t iotex_t_number;
typedef uint32_t iotex_t_boolean;

typedef enum {
    IOTEX_E_NOERR,
    IOTEX_E_VER,
    IOTEX_E_CERT,
    IOTEX_E_URL,
    IOTEX_E_PRVKEY,
    IOTEX_E_MEM,
    IOTEX_E_REQUEST,
    IOTEX_E_PARSE,
    IOTEX_E_RESPONSE,
    IOTEX_E_UNSUPPORT,
    IOTEX_E_PBPACK,
    IOTEX_E_HEX2STR
} iotex_em_error;

/* Structure for configure cert and api version  */
typedef struct iotex_st_config {
    uint32_t ver;		// pharos API version, default 1
    long verify_cert;		// set 1 verify the pharos SSL certificate, 0 don't verify SSL certificate
    long verify_host;		// set 2 verify the certificate's name against host, 0 don't verify
    const char *cert_dir;	// SSL certificate directory, set NULL will auto search (embedded linux may not works fine)
    const char *cert_file;	// SSL certificate file, set NULL will auto search (embedded linux may not works fine)
} iotex_st_config;

typedef struct {
    iotex_t_account address;
    iotex_t_number balance;
    iotex_t_number nonce;
    iotex_t_number pendingNonce;
    iotex_t_number numActions;
} iotex_st_account_meta;

typedef struct {
    uint64_t height;
    uint64_t numActions;
    uint64_t tps;
    struct {
        uint64_t num;
        uint64_t height;
        uint64_t gravityChainStartHeight;
    } epoch;
    double tpsFloat;
} iotex_st_chain_meta;

typedef struct iotex_st_action_info {
    struct {
        struct {
            iotex_t_number nonce;
            iotex_t_number version;
            iotex_t_number gasLimit;
            iotex_t_number gasPrice;
            struct {
                iotex_t_number amount;
                iotex_t_account recipient;
            } transfer;
        } core;
        iotex_t_key senderPubKey;
        iotex_t_key signature;
    } action;

    iotex_t_hash actHash;
    iotex_t_hash blkHash;
    iotex_t_number blkHeight;
    iotex_t_account sender;
    iotex_t_number gasFee;
    iotex_t_time timestamp;
} iotex_st_action_info;

typedef struct {
    char data[2048];
    size_t size;
} iotex_st_contract_data;

typedef struct {
    iotex_t_id id;
    iotex_t_boolean status;
    struct {
        struct {
            iotex_t_number annual;
        } reward;

        iotex_t_number locktime;
        iotex_t_number minimum_amount;
    } details;
} iotex_st_validator;

typedef struct {
    const uint64_t *version;
    const uint64_t *nonce;
    const uint64_t *gasLimit;
    const char *gasPrice;
    const char *privateKey;
} iotex_st_act_core;

typedef struct iotex_st_transfer {
    const char *amount;
    const char *recipient;
    const uint8_t *payload;
    size_t payloadLength;
    iotex_st_act_core core;
} iotex_st_transfer;

typedef struct iotex_st_execution {
    const char *amount;
    const char *contract;
    const uint8_t *data;
    size_t dataLength;
    iotex_st_act_core core;
} iotex_st_execution;

/*
 * @brief: iotex_emb api initialize, configure cert and api version etc
 * #config: specify cert file or directory, set NULL use the system default
 * $return: success return 0, failed return negative error code(iotex_em_error)
 */
int iotex_emb_init(const iotex_st_config *config);

/*
 * @brief: clear api configure
 */
void iotex_emb_exit();

/*
 * @brief: get blockchain metadata
 * #chain_meta: structure pointer to save blockchain metadata
 * $return: success return 0, failed return negative error code(iotex_em_error)
 */
int iotex_emb_get_chain_meta(iotex_st_chain_meta *chain_meta);

/*
 * @brief: get account metadata
 * #account: account encoded address
 * #meta: structure pointer to save account metadata
 * $return: success return 0, failed return negative error code(iotex_em_error)
 */
int iotex_emb_get_account_meta(const char *account, iotex_st_account_meta *meta);

/*
 * @brief: get transfer action by block height
 * #block: block height
 * #action: structure pointer to iotex_st_action_info, save action information
 * $return: success return 0, failed return negative error code(iotex_em_error)
 */
int iotex_emb_get_transfer_block(uint128_t block, iotex_st_action_info *action);

/*
 * @brief: get action by action hash
 * #hash: hash of action
 * #action: structure pointer to iotex_st_action_info, save action information
 * $return: success return 0, failed return negative error code(iotex_em_error)
 */
int iotex_emb_get_action_by_hash(const char *hash, iotex_st_action_info *action);

/*
 * @brief: get actions by address
 * #addr: encoded address
 * #start_idx: start index of actions
 * #count: number of actions to get
 * #actions: iotex_st_action_info array to save actions list
 * #max_size: #actions array size
 * #actual_size: return actions list actual size
 * $return: success return 0, failed return negative error code(iotex_em_error)
 */
int iotex_emb_get_action_by_addr(const char *addr,
                                 uint32_t start_idx, uint32_t count,
                                 iotex_st_action_info *actions, size_t max_size, size_t *actual_size);

/*
 * @brief: get contract data by address
 * #addr: encoded address
 * #method: hex-encoded contract func signature
 * #data: hex-encoded calldata
 * $return: success return 0, failed return negative error code(iotex_em_error)
 */
int iotex_emb_read_contract_by_addr(const char *addr,
    const char *method, const char *data, iotex_st_contract_data *contract_data);

/*
 * @brief: get validator members list
 * #validators: iotex_st_validator array to save validator list
 * #max_size: #validators array max size
 * #actual_size: return validator members list actual size
 * $return: success return 0, failed return negative error code(iotex_em_error)
 */
int iotex_emb_get_validators(iotex_st_validator *validators, size_t max_size, size_t *actual_size);

/*
 * @brief: send a transaction of transfer to iotex blockchain network
 * #transfer: transfer parameters
 * #action_hash: success return transfer action hash
 * #error_desc: successed NULL failed strndup error desc
 * $return: success return 0, failed return negative error code(iotex_em_error)
 */
int iotex_emb_transfer(const iotex_st_transfer *transfer, iotex_t_hash hash, char **error_desc);
int iotex_emb_execution(const iotex_st_execution *execution, iotex_t_hash hash, char **error_desc);

#ifdef	__cplusplus
}
#endif


#endif /* _IOTEX_EMB_H_ */
