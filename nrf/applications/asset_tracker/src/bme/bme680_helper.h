#ifndef _IOTEX_BME680_H_
#define _IOTEX_BME680_H_

#include <stdint.h>
#include "nvs/local_storage.h"

#define I2C_DEV_BME680    "I2C_2"
#define I2C_ADDR_BME680   0x77


int iotex_bme680_init(void);
int iotex_bme680_get_sensor_data(iotex_storage_bme680 *bme680);

#endif //_IOTEX_BME680_H_