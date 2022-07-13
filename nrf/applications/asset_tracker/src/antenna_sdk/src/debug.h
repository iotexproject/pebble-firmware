#ifndef _IOTEX_EMB_DEBUG_H_
#define _IOTEX_EMB_DEBUG_H_

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>


#ifdef _DEBUG_
#define __INFO_MSG__(mesg) do { fprintf(stdout, "%s[%s] --> [%d] --> %s\n", __FILE__,__func__, __LINE__, mesg);} while(0)
#define __WARN_MSG__(mesg) do { fprintf(stderr, "%s[%s] --> [%d] --> %s\n", __FILE__,__func__, __LINE__, mesg);} while(0)
#define __ERROR_MSG__(mesg) do { fprintf(stderr, "%s[%s] --> [%d] --> %s: %s\n", __FILE__,__func__, __LINE__, mesg, strerror(errno));} while(0)
#else
#define __INFO_MSG__(mesg) do { fprintf(stdout, "%s\n", mesg);} while(0)
#define __WARN_MSG__(mesg) do { fprintf(stderr, "%s\n", mesg);} while(0)
#define __ERROR_MSG__(mesg) do { fprintf(stderr, "%s: %s\n", mesg, strerror(errno));} while(0)
#endif



#ifdef	__cplusplus
}
#endif

#endif /* _IOTEX_EMB_DEBUG_H_ */

