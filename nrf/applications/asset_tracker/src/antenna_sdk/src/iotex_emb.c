#include <assert.h>
#include "u128.h"
#include "rule.h"
#include "debug.h"
#include "parse.h"
#include "signer.h"
#include "debug.h"
#include "config.h"
#include "request.h"
#include "response.h"
#include "pb_proto.h"
#include "iotex_emb.h"

int iotex_emb_init(const iotex_st_config *config) {
    return init_config(config);
}

void iotex_emb_exit() {
    clear_config();
}

int iotex_emb_get_chain_meta(iotex_st_chain_meta *chain) {

    assert(chain != NULL);

    json_parse_rule epoch_rules[] = {
        {"num", JSON_TYPE_NUMBER64, NULL, (void *) &chain->epoch.num},
        {"height", JSON_TYPE_NUMBER64, NULL, (void *) &chain->epoch.height},
        {"gravityChainStartHeight", JSON_TYPE_NUMBER64, NULL, (void *) &chain->epoch.gravityChainStartHeight},
        {NULL}
    };

    json_parse_rule chain_meta_rules[] = {

        {"height", JSON_TYPE_NUMBER64, NULL, (void *) &chain->height},
        {"numActions", JSON_TYPE_NUMBER64, NULL, (void *) &chain->numActions},
        {"tps", JSON_TYPE_NUMBER64, NULL, (void *) &chain->tps},
        {"epoch", JSON_TYPE_OBJECT, epoch_rules},
        {"tpsFloat", JSON_TYPE_DOUBLE, NULL, (void *) &chain->tpsFloat},
        {NULL,}
    };

    char url[IOTEX_EMB_MAX_URL_LEN];

    if (!req_compose_url(url, sizeof(url), REQ_GET_CHAINMETA)) {
        __WARN_MSG__("compose url failed!");
        return -IOTEX_E_URL;
    }

    return res_get_data(url, chain_meta_rules);
}

int iotex_emb_get_account_meta(const char *account, iotex_st_account_meta *meta) {

    assert(account != NULL);
    assert(meta != NULL);

    json_parse_rule account_meta[] = {
        {"address", JSON_TYPE_STR, NULL, (void *) &meta->address, sizeof(meta->address)},
        {"balance", JSON_TYPE_NUMBER, NULL, (void *) &meta->balance},
        {"nonce", JSON_TYPE_NUMBER, NULL, (void *) &meta->nonce},
        {"pendingNonce", JSON_TYPE_NUMBER, NULL, (void *) &meta->pendingNonce},
        {"numActions", JSON_TYPE_NUMBER, NULL, (void *) &meta->numActions},
        {NULL}
    };

    json_parse_rule account_rules[] = {
        {"accountMeta", JSON_TYPE_OBJECT, account_meta},
        {NULL}
    };

    char url[IOTEX_EMB_MAX_URL_LEN];

    if (!req_compose_url(url, sizeof(url), REQ_GET_ACCOUNT, account)) {
        __WARN_MSG__("compose url failed!");
        return -IOTEX_E_URL;
    }

    return res_get_data(url, account_rules);
}

int iotex_emb_get_transfer_block(uint128_t block, iotex_st_action_info *action) {

    assert(action != NULL);

    int ret;
    char url[IOTEX_EMB_MAX_URL_LEN];
    char block_str[UINT128_RAW_MAX_LEN];

    /* Block height must convert to string, in case vsnprintf can't handle */
    if (!req_compose_url(url, sizeof(url), REQ_GET_TRANSFERS_BY_BLOCK, u1282str(block, block_str, sizeof(block_str)))) {
        __WARN_MSG__("compose url failed!");
        return -IOTEX_E_URL;
    }

    if ((ret = res_get_actions(url, action, 1)) != 1) {
        return ret;
    }

    return 0;
}

int iotex_emb_get_action_by_hash(const char *hash, iotex_st_action_info *action) {

    assert(hash != NULL);
    assert(action != NULL);

    int ret;
    char url[IOTEX_EMB_MAX_URL_LEN];

    if (!req_compose_url(url, sizeof(url), REQ_GET_ACTIONS_BY_HASH, hash)) {
        __WARN_MSG__("compose url failed!");
        return -IOTEX_E_URL;
    }

    if ((ret = res_get_actions(url, action, 1)) != 1) {
        return ret;
    }

    return 0;
}

int iotex_emb_get_action_by_addr(const char *addr,
                                 uint32_t start_idx, uint32_t count,
                                 iotex_st_action_info *actions, size_t max_size, size_t *actual_size) {

    assert(addr != NULL);
    assert(actions != NULL);

    int ret;
    char url[IOTEX_EMB_MAX_URL_LEN];

    if (!req_compose_url(url, sizeof(url), REQ_GET_ACTIONS_BY_ADDR, addr, start_idx, count)) {
        __WARN_MSG__("compose url failed!");
        return -IOTEX_E_URL;
    }

    if ((ret = res_get_actions(url, actions, max_size)) < 0) {
        return ret;
    }

    if (actual_size) {
        *actual_size = ret;
    }

    return 0;
}

int iotex_emb_read_contract_by_addr(const char *addr,
    const char *method, const char *data, iotex_st_contract_data *contract_data) {

    assert(addr != NULL);
    assert(method != NULL);
    assert(data != NULL);
    assert(contract_data != NULL);

    int ret;
    char url[IOTEX_EMB_MAX_URL_LEN];

    if (!req_compose_url(url, sizeof(url), REQ_READ_CONTRACT_BY_ADDR, addr, method, data)) {
        __WARN_MSG__("compose url failed!");
        return -IOTEX_E_URL;
    }

    if ((ret = res_get_contract_data(url, contract_data)) < 0) {
        return ret;
    }

    // contract_data->data is hex-encoded string, convert back to bytes
    size_t size = strlen(contract_data->data)/2;
    if ((ret = signer_str2hex(contract_data->data, (uint8_t *)contract_data->data, size)) < 0) {
        return ret;
    }

    contract_data->size = size;
    return 0;
}

int iotex_emb_get_validators(iotex_st_validator *validators, size_t max_size, size_t *actual_size) {

    assert(validators != NULL);

    char url[IOTEX_EMB_MAX_URL_LEN];

    json_parse_rule reward_rule[] = {
        {"annual", JSON_TYPE_NUMBER},
        {NULL},
    };

    json_parse_rule details_rule[] = {
        {"locktime", JSON_TYPE_NUMBER},
        {"minimum_amount", JSON_TYPE_NUMBER},
        {"reward", JSON_TYPE_OBJECT, reward_rule},
        {NULL},
    };

    json_parse_rule validator_rule[] = {
        {"id", JSON_TYPE_STR},
        {"status", JSON_TYPE_BOOLEAN},
        {"details", JSON_TYPE_OBJECT, details_rule},
        {NULL},
    };

    json_parse_rule rule = {
        NULL, JSON_TYPE_ARRAY, validator_rule,
        validators, max_size, JSON_TYPE_OBJECT,
        sizeof(iotex_st_validator), rule_validator_bind, actual_size
    };

    if (!req_compose_url(url, sizeof(url), REQ_GET_MEMBER_VALIDATORS)) {
        __WARN_MSG__("compose url failed!");
        return -IOTEX_E_URL;
    }

    return res_get_data(url, &rule);
}

int iotex_emb_transfer(const iotex_st_transfer *transfer, iotex_t_hash hash, char **error_desc) {

    assert(transfer != NULL);

    if (!transfer->core.privateKey) {
        return -IOTEX_E_PRVKEY;
    }

    return res_get_hash((void *)transfer, ACT_TRANSFER,  hash, IOTEX_EMB_LIMIT_HASH_LEN, error_desc);
}

int iotex_emb_execution(const iotex_st_execution *execution, iotex_t_hash hash, char **error_desc) {

    assert(execution != NULL);

    if (!execution->core.privateKey) {
        return -IOTEX_E_PRVKEY;
    }

    return res_get_hash((void *)execution, ACT_EXECUTION, hash, IOTEX_EMB_LIMIT_HASH_LEN, error_desc);
}
