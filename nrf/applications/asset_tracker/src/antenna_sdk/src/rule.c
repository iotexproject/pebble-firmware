#include <assert.h>
#include "rule.h"
#include "debug.h"
#include "iotex_emb.h"

typedef struct {
    /* Rule key to find rule */
    const char *key;
    /* data bind to key matched rule */
    void *data;
    /* data size, only works for array(array size) or string buffer(in bytes)  */
    size_t size;
} st_rule_data_bind;

/*
 * @brief: find a json_parse_rule by rule.key
 * #rule: rule chain to find
 * #key: key to find
 * $return: successed retrun matched rule, failed retrun NULL
 */
json_parse_rule *find_rule_by_key(json_parse_rule *rule, const char *key) {

    if (!rule || !key) {
        return NULL;
    }

    while (rule && rule->key) {

        if (strncmp(rule->key, key, strlen(key)) == 0) {
            return rule;
        }

        rule++;
    }

    return NULL;
}

json_parse_rule *find_sub_rule_by_key(json_parse_rule *rule, const char *key) {

    json_parse_rule *found = find_rule_by_key(rule, key);

    if (found && found->sub) {
        return found->sub;
    }

    return NULL;
}

/*
 * @brief: bind rule and data
 * #rule: top rule
 * #bind: st_rule_data_bind include rule key, rule data and data size
 */
static void bind_rule_and_data(json_parse_rule *rule, const st_rule_data_bind *bind) {

    json_parse_rule *rule_to_bind = NULL;

    while (bind && bind->key) {

        if ((rule_to_bind = find_rule_by_key(rule, bind->key))) {
            rule_to_bind->value = bind->data;
            rule_to_bind->value_len = bind->size;
        }

        bind++;
    }
}

/*
 * @brief: bind iotex_st_action_info parse rule to data
 * #rule_chain: rule chain to parse iotex_st_action_info
 * #element: iotex_st_action_info pointer
 * $return: success bind retrun 0, failed return -1;
 */
int rule_action_info_bind(json_parse_rule *rule_chain, void *element) {

    if (!rule_chain || !element) {

        return -1;
    }

    iotex_st_action_info *action_info = (iotex_st_action_info *)element;
    json_parse_rule *action_rule = find_sub_rule_by_key(rule_chain, "action");
    json_parse_rule *action_core_rule = find_sub_rule_by_key(action_rule, "core");
    json_parse_rule *core_transfer_rule = find_sub_rule_by_key(action_core_rule, "transfer");

    if (!action_rule || !action_core_rule || !core_transfer_rule) {
        return -1;
    }

    st_rule_data_bind info[] = {
        {"actHash", (void *) &action_info->actHash, sizeof(action_info->actHash)},
        {"blkHash", (void *) &action_info->blkHash, sizeof(action_info->blkHash)},
        {"blkHeight", (void *) &action_info->blkHeight},
        {"sender", (void *) &action_info->sender, sizeof(action_info->sender)},
        {"gasFee", (void *) &action_info->gasFee},
        {"timestamp", (void *) &action_info->timestamp, sizeof(action_info->timestamp)},
        {NULL}
    };

    st_rule_data_bind action[] = {
        {"senderPubKey", (void *) &action_info->action.senderPubKey, sizeof(action_info->action.senderPubKey)},
        {"signature", (void *) &action_info->action.signature, sizeof(action_info->action.signature)},
        {NULL}
    };

    st_rule_data_bind action_core[] = {
        {"nonce", (void *) &action_info->action.core.nonce},
        {"version", (void *) &action_info->action.core.version},
        {"gasLimit", (void *) &action_info->action.core.gasLimit},
        {"gasPrice", (void *) &action_info->action.core.gasPrice},
        {NULL}
    };

    st_rule_data_bind core_transfer[] = {
        {"amount", (void *) &action_info->action.core.transfer.amount},
        {"recipient", (void *) &action_info->action.core.transfer.recipient, sizeof(action_info->action.core.transfer.recipient)},
        {NULL}
    };

    /* Bind data and rule */
    bind_rule_and_data(rule_chain, info);
    bind_rule_and_data(action_rule, action);
    bind_rule_and_data(action_core_rule, action_core);
    bind_rule_and_data(core_transfer_rule, core_transfer);

    return 0;
}


int rule_validator_bind(json_parse_rule *rule_chain, void *element) {

    if (!rule_chain || !element) {
        return -1;
    }

    iotex_st_validator *validator = (iotex_st_validator *)element;
    json_parse_rule *details_rule = find_sub_rule_by_key(rule_chain, "details");
    json_parse_rule *details_reward_rule = find_sub_rule_by_key(details_rule, "reward");

    if (!details_rule || !details_reward_rule) {
        return -1;
    }

    st_rule_data_bind info[] = {
        {"id", (void *) &validator->id, sizeof(validator->id)},
        {"status", (void *) &validator->status},
        {NULL}
    };

    st_rule_data_bind details[] = {
        {"locktime", (void *) &validator->details.locktime},
        {"minimum_amount", (void *) &validator->details.minimum_amount},
        {NULL}
    };

    st_rule_data_bind details_reward[] = {
        {"annual", (void *) &validator->details.reward},
        {NULL}
    };

    bind_rule_and_data(rule_chain, info);
    bind_rule_and_data(details_rule, details);
    bind_rule_and_data(details_reward_rule, details_reward);

    return 0;
}
