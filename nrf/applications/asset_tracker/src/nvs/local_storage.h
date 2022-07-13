#ifndef _IOTEX_STORAGE_H_
#define _IOTEX_STORAGE_H_

#include <stdint.h>
#include <stdio.h>

typedef enum {
    SID_DEVICE_GPS = 0x1,
    SID_DEVICE_INFO = 0x2,

    SID_SENSOR_GAS = 0X10,
    SID_SENSOR_TEMP = 0x20,
    SID_SENSOR_PRESSURE = 0x40,
    SID_SENSOR_HUMIDITY = 0X80,
    SID_SENSOR_BME680_ALL = 0xf0,

    SID_SENSOR_TEMP2 = 0x100,
    SID_SENSOR_GYROSCOPE = 0x200,
    SID_SENSOR_ACCELEROMETER = 0x400,
    SID_SENSOR_CUSTOM_MOTION = 0x800,
    SID_SENSOR_ICM42605_ALL = 0xf00,

    /* Sampling data sid */
    SID_MQTT_BULK_UPLOAD_DATA = 0x1000,

    /* MQTT configure sid */
    SID_MQTT_DATA_CHANNEL_CONFIG = 0x1001,
	SID_ECC_KEY = 0x1002,
} iotex_storage_id;


/* GPS data */
typedef struct {
    float latitude;
    float longutude;
    uint64_t timestamp;
} iotex_storage_gps;

/* Device info */
typedef struct {
    int8_t snr;
    int8_t resv;
    uint16_t vbatx100;
} iotex_storage_devinfo;

/* BME680 Sensor */
typedef struct {
    float pressure;
    float humidity;
    float temperature;
    float gas_resistance;
} iotex_storage_bme680;

/* ICM42605 Sensor */
typedef struct {
    float temperature;
    int16_t gyroscope[3];
    int16_t accelerometer[3];
} iotex_storage_icm42605;


int iotex_local_storage_test(void);
int iotex_local_storage_init(void);

int iotex_local_storage_del(iotex_storage_id id);
int iotex_local_storage_load(iotex_storage_id id, void *data, size_t len);
int iotex_local_storage_save(iotex_storage_id id, const void *data, size_t len);
int iotex_local_storage_hist(iotex_storage_id id, void *data, size_t len, uint16_t cnt);
int iotex_local_storage_readall(iotex_storage_id id, void *data, size_t size, size_t item_len);

#endif //_IOTEX_STORAGE_H_
