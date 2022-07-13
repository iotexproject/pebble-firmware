#include <stdlib.h>
#include <string.h>
#include "rule.h"
#include "debug.h"
#include "parse.h"
#include "config.h"
#include "signer.h"
#include "request.h"
#include "response.h"
#include "pb_proto.h"
#include "iotex_emb.h"


#define CLR_ERROR_DESC(desc) do {if (desc) *desc = NULL;} while (0)
#define SET_ERROR_DESC(error, desc) do {if (desc && error) *desc = strndup(error, strlen(error));} while (0)


/*
 * @brief: send request to http server and get response data then follow the rule parse json string to struct data
 * #request: http request should include base url and post data
 * #rule: json_parse_rule tell the parser how to parse response data
 * $return: successed return 0, (the data will saved to the rule binded struct), failed return negative value
 */
int res_get_data(const char *request, json_parse_rule *rules) {
    char *response = NULL;

    if ((response = malloc(IOTEX_EBM_MAX_RES_LEN)) == NULL) {
        return -IOTEX_E_MEM;
    }

#ifdef _DEBUG_HTTP_
    __INFO_MSG__(request);
#endif

    if (req_get_request(request, response, IOTEX_EBM_MAX_RES_LEN) != 0) {
        free(response);
        return -IOTEX_E_REQUEST;
    }

#ifdef _DEBUG_HTTP_
    __INFO_MSG__(response);
#endif

    if (json_parse_response(response, rules) != 0) {
        free(response);
        return -IOTEX_E_PARSE;
    }

    free(response);
    return 0;
}

int res_get_hash(const void *action, uint8_t action_id, char *hash, size_t hash_max, char **error_desc) {
    int action_bytes_len;
    char *response = NULL;
    char request[IOTEX_EMB_MAX_URL_LEN + IOTEX_EMB_MAX_ACB_LEN * 2];

    uint8_t action_bytes[IOTEX_EMB_MAX_ACB_LEN];
    char action_bytes_str[IOTEX_EMB_MAX_ACB_LEN * 2] = {0};

    /* Response data is a json {"actionHash": "xxxxxx"} */
    json_parse_rule response_purse_rule[] = {
        {"actionHash", JSON_TYPE_STR, NULL, (void *)hash, hash_max},
        {NULL}
    };

    /* Generate tx action bytes */
    switch (action_id) {
        case ACT_TRANSFER:
            action_bytes_len = proto_gen_tx_action((const iotex_st_transfer *)action, action_bytes, sizeof(action_bytes));
            break;

        case ACT_EXECUTION:
            action_bytes_len = proto_gen_ex_action((const iotex_st_execution *)action, action_bytes, sizeof(action_bytes));
            break;

        default:
            SET_ERROR_DESC("Unsupported action", error_desc);
            return -IOTEX_E_UNSUPPORT;
    }

    if (action_bytes_len <= 0) {
        return -IOTEX_E_PBPACK;
    }

    /* Convert action bytes to string */
    if (signer_hex2str(action_bytes, action_bytes_len, action_bytes_str, sizeof(action_bytes_str)) < 0) {
        return -IOTEX_E_HEX2STR;
    }

    /* Compose url*/
    if (!req_compose_url(request, sizeof(request), REQ_SEND_SIGNED_ACTION_BYTES, action_bytes_str)) {
        return -IOTEX_E_URL;
    }

    /* Malloc response data buffer */
    if ((response = malloc(IOTEX_EBM_MAX_RES_LEN)) == NULL) {
        return -IOTEX_E_MEM;
    }

#ifdef _DEBUG_HTTP_
    __INFO_MSG__("Request:");
    __INFO_MSG__(request);
#endif

    if (req_post_request(request, response, IOTEX_EBM_MAX_RES_LEN) != 0) {
        free(response);
        return -IOTEX_E_REQUEST;
    }

#ifdef _DEBUG_HTTP_
    __INFO_MSG__("Response:");
    __INFO_MSG__(response);
#endif

    CLR_ERROR_DESC(error_desc);
    if (json_parse_response(response, response_purse_rule) != 0) {
        free(response);
        return -IOTEX_E_RESPONSE;
    }

    free(response);
    return 0;
}


/*
 * @brief: get actions info list
 * #request: http request should include base url and post data
 * #actions: an array to save action
 * #max_size: #actions array size
 * $return: successed return actual get actions size, failed -1
 */
int res_get_actions(const char *request, iotex_st_action_info *actions, size_t max_size) {

    int ret;
    int32_t actual_size = 0;

    json_parse_rule core_transfer_rule[] = {
        {"amount", JSON_TYPE_NUMBER},
        {"recipient", JSON_TYPE_STR},
        {NULL}
    };

    json_parse_rule action_core_rule[] = {
        {"nonce", JSON_TYPE_NUMBER},
        {"version", JSON_TYPE_NUMBER},
        {"gasLimit", JSON_TYPE_NUMBER},
        {"gasPrice", JSON_TYPE_NUMBER},
        {"transfer", JSON_TYPE_OBJECT, core_transfer_rule},
        {NULL}
    };

    json_parse_rule action_rule[] = {
        {"signature", JSON_TYPE_STR},
        {"senderPubKey", JSON_TYPE_STR},
        {"core", JSON_TYPE_OBJECT, action_core_rule},
        {NULL}
    };

    json_parse_rule action_info_rule[] = {
        {"action", JSON_TYPE_OBJECT, action_rule},
        {"actHash", JSON_TYPE_STR},
        {"blkHash", JSON_TYPE_STR},
        {"blkHeight", JSON_TYPE_NUMBER},
        {"sender", JSON_TYPE_STR},
        {"gasFee", JSON_TYPE_NUMBER},
        {"timestamp", JSON_TYPE_STR},
        {NULL}
    };

    json_parse_rule top_rule[] = {
        {"total", JSON_TYPE_NUMBER32, NULL, (void *) &actual_size},
        /* key, json value type, array element parse rule, an array to save object, object array size, array element type, single object size, rule data bind callback */
        {"actionInfo", JSON_TYPE_ARRAY, action_info_rule, (void *)actions, max_size, JSON_TYPE_OBJECT, sizeof(iotex_st_action_info), rule_action_info_bind},
        {NULL}
    };

    if ((ret = res_get_data(request, top_rule)) != 0) {
        fprintf(stderr, "get actions failed: %d\n", ret);
        return ret;
    }

    return actual_size;
}

int res_get_contract_data(const char *request, iotex_st_contract_data *contract_data) {
    json_parse_rule contract_rules[] = {
        {"data", JSON_TYPE_STR, NULL, (void *)contract_data->data, sizeof(contract_data->data)},
        {NULL}
    };

    return res_get_data(request, contract_rules);
}
