#ifndef _INV_ICM426XX_ACTION_DETECT_H_
#define _INV_ICM426XX_ACTION_DETECT_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "Icm426xxDefs.h"
#include "Icm426xxTransport.h"
#include "Icm426xxDriver_HL.h"

enum  SENSOR_ACTION
{
    ACT_TILT,
    ACT_WOM,
    ACT_SMD,
    ACT_RAW,
    MAX_ACT
};

#define   ACTION_DETECTED   1
#define   ACTION_NOT_DETECTED  0

int icm426xxConfig(enum SENSOR_ACTION  act);
int icm426xxActionDetect(enum SENSOR_ACTION  act);

#ifdef __cplusplus
}
#endif

#endif
