#ifndef _IOTEX_ICM42605_H_
#define _IOTEX_ICM42605_H_

#include <stdint.h>
#include "nvs/local_storage.h"

#define I2C_DEV_ICM42605  "I2C_1"
#define I2C_ADDR_ICM42605 0x69

#define IS_LOW_NOISE_MODE 1
#define TMST_CLKIN_32K 0

#define SERIF_TYPE ICM426XX_UI_I2C

int iotex_icm42605_init(void);
int iotex_icm42605_get_sensor_data(iotex_storage_icm42605 *icm42605);


/* For Icm426xxDriver_HL using */
void inv_icm426xx_sleep_us(uint32_t us);
uint64_t inv_icm426xx_get_time_us(void);
struct inv_icm426xx * getICMDriver(void);

#endif
