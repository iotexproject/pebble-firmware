#include <zephyr.h>
#include <stdio.h>
#include <stdlib.h>
#include <drivers/i2c.h>
#include "Icm426xxDefs.h"
#include "Icm426xxTransport.h"
#include "Icm426xxDriver_HL.h"
#include "icm42605_helper.h"
#include "modem/modem_helper.h"
#include "nvs/local_storage.h"
#include "icm426xxActionDetect.h"

extern int tiltConf(void);
extern int womConf(void);
extern int smdConf(void);
extern int rawdataConf(void);

extern int tiltDetect(void);
extern int womDetect(void);
extern int smdDetect(void);
extern int  rawdataDetect(void);

const static int (*sensorConfig[MAX_ACT][2])(void)={
    { tiltConf, tiltDetect },
    { womConf, womDetect },
    { smdConf, smdDetect },
    { rawdataConf, rawdataDetect}
};


int icm426xxConfig(enum SENSOR_ACTION  act)
{
    return (*sensorConfig[act][0])();
}

int icm426xxActionDetect(enum SENSOR_ACTION  act)
{
    return (*sensorConfig[act][1])();
}

