#ifndef  _IOTEX_MQTT_CONFIG_H_
#define  _IOTEX_MQTT_CONFIG_H_

#include <stdint.h>

/*
  Json configure example:

  {
    "bulk_upload": 1,
    "data_channel": 38,
    "upload_period": 10,
    "bulk_upload_sampling_cnt": 60,
    "bulk_upload_sampling_freq": 10
  }
*/

typedef struct {
    /* Enabled bulk upload */
    uint32_t bulk_upload;

    /* Data of interested, group of iotex_data_channel */
    uint16_t data_channel;
    /* Periodically upload, upload period, zero means turn off */
    uint16_t upload_period;

    /* Bulk upload, data will store to local storage,
       until sampling count equal to bulk_upload_sampling_cnt,
       bulk upload all sampling data.

       Take notic of local storage capacity,
       bulk_upload_sampling_cnt * bulk_upload_sampling_freq
       should less than maximum of local storage volume.
    */
    uint16_t bulk_upload_sampling_cnt;
    /* Bulk upload sampling frequency */
    uint16_t bulk_upload_sampling_freq;

    /* Record current data upload count, when all data uploaded enable sampling mode */
    uint16_t current_upload_cnt;

    /* Record current sampling data count, when sampling count is fulfilled enable upload mode */
    uint16_t current_sampling_cnt;
	// bytes of once nvs_wrtie
	uint32_t  size_of_block;
} iotex_mqtt_config;

typedef enum {
    DATA_CHANNEL_GPS = 0x1,
    DATA_CHANNEL_SNR = 0x2,
    DATA_CHANNEL_VBAT = 0x4,
    DATA_CHANNEL_DEVINFO = 0x6,

    DATA_CHANNEL_GAS = 0X10,
    DATA_CHANNEL_TEMP = 0x20,
    DATA_CHANNEL_PRESSURE = 0x40,
    DATA_CHANNEL_HUMIDITY = 0X80,
    DATA_CHANNEL_ENV_SENSOR = 0xf0,

    DATA_CHANNEL_TEMP2 = 0x100,
    DATA_CHANNEL_GYROSCOPE = 0x200,
    DATA_CHANNEL_ACCELEROMETER = 0x400,
    DATA_CHANNEL_CUSTOM_MOTION = 0x800,
    DATA_CHANNEL_ACTION_SENSOR = 0xf00,
    DATA_CHANNEL_LIGHT_SENSOR = 0x1000,
} iotex_data_channel;


#define IOTEX_DATA_CHANNEL_IS_SET(x, c)   (((x) & (c)) == (c))

bool iotex_mqtt_is_bulk_upload(void);
bool iotex_mqtt_is_need_sampling(void);

uint16_t iotex_mqtt_get_data_channel(void);
uint16_t iotex_mqtt_get_upload_period(void);
uint16_t iotex_mqtt_get_bulk_upload_count(void);
uint16_t iotex_mqtt_get_sampling_frequency(void);

/* Increase upload count,
   return true means all of local nvs stored sampling data is uploaded.
   Auto switch to sampling mode.
*/
bool iotex_mqtt_inc_current_upload_count(void);


/* Increase sampling count,
   return true means required sampling count is fulfilled, it't ready to upload.
   Auto switch to upload mode
*/
bool iotex_mqtt_inc_current_sampling_count(void);

uint16_t iotex_mqtt_get_current_upload_count(void);
uint16_t iotex_mqtt_get_current_sampling_count(void);

void iotex_mqtt_load_config(void);
void iotex_mqtt_update_config(const uint8_t *payload, uint32_t len);
//bool iotex_mqtt_parse_config(const uint8_t *payload, uint32_t len, iotex_mqtt_config *config);
void set_block_size(uint32_t size);
uint32_t get_block_size(void);
uint16_t get_his_block(void);
bool iotex_mqtt_is_bulk_upload_over(void);
void config_mutex_lock(void);
void config_mutex_unlock(void);

#endif /* _IOTEX_MQTT_CONFIG_H_ */