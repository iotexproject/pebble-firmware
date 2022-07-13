#ifndef _IOTEX_EMB_UTILS_H_
#define _IOTEX_EMB_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

/* rau ==> iotx, failed return NULL */
const char *utils_rau2iotx(const char *rau, char *iotx, size_t max);

#ifdef __cplusplus
}
#endif

#endif /* _IOTEX_EMB_UTILS_H_ */

