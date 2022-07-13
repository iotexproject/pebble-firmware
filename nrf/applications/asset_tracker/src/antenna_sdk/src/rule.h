#ifndef _IOTEX_EMB_RULE_H_
#define _IOTEX_EMB_RULE_H_

#ifdef	__cplusplus
extern "C" {
#endif

#include "parse.h"

int rule_validator_bind(json_parse_rule *rule_chain, void *element);
int rule_action_info_bind(json_parse_rule *rule_chain, void *element);

json_parse_rule *find_rule_by_key(json_parse_rule *rule, const char *key);
json_parse_rule *find_sub_rule_by_key(json_parse_rule *rule, const char *key);

#ifdef	__cplusplus
}
#endif

#endif /* _IOTEX_EMB_RULE_H_ */
