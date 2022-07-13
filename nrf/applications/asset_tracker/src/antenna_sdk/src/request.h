#ifndef _IOTEX_EMB_REQUEST_H_
#define _IOTEX_EMB_REQUEST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum {

    REQ_GET_ACCOUNT,
    REQ_GET_CHAINMETA,
    REQ_GET_TRANSFERS_BY_BLOCK,

    REQ_GET_ACTIONS_BY_HASH,
    REQ_GET_ACTIONS_BY_ADDR,
    REQ_READ_CONTRACT_BY_ADDR,

    REQ_GET_MEMBER_VALIDATORS,
    REQ_GET_MEMBER_DELEGATIONS,

    REQ_SEND_SIGNED_ACTION_BYTES,

    REQ_TAIL_NONE
} iotex_em_request;


char *req_compose_url(char *url, size_t url_max_size, iotex_em_request req, ...);
int req_get_request(const char *request, char *response, size_t response_max_size);
int req_post_request(const char *request, char *response, size_t response_max_size);

#ifdef __cplusplus
}
#endif

#endif /* _IOTEX_EMB_REQUEST_H_ */
