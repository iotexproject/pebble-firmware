#ifndef  _IOTEX_MQTT_H_
#define  _IOTEX_MQTT_H_

#include <net/mqtt.h>
#include <net/cloud.h>
#include <zephyr.h>
#include <net/socket.h>

extern atomic_val_t send_data_enable;

struct mqtt_payload {
    char *buf;
    size_t len;
};

/* Get MQTT client id */
const uint8_t *iotex_mqtt_get_client_id(void);

bool iotex_mqtt_is_connected(void);
int iotex_mqtt_client_init(struct mqtt_client *client, struct pollfd *fds);
int iotex_mqtt_publish_data(struct mqtt_client *client, enum mqtt_qos qos, char *data);

/* Sampling data and pack to json string to output buffer */
int iotex_mqtt_get_devinfo_payload(struct mqtt_payload *output);
int iotex_mqtt_get_env_sensor_payload(struct mqtt_payload *output);
int iotex_mqtt_get_action_sensor_payload(struct mqtt_payload *output);

/* Bulk upload nvs stored data */
void iotex_mqtt_bulk_upload_sampling_data(uint16_t channel);

/* Sampling and store data to nvs */
bool iotex_mqtt_sampling_data_and_store(uint16_t channel);

/* Sampling data and pack seleccted data to json string to output buffer */
int iotex_mqtt_get_selected_payload(uint16_t channel, struct mqtt_payload *output);
//  wrap  json package
int iotex_mqtt_bin_to_json(uint8_t *buffer, uint16_t channel, struct mqtt_payload *output);
// mqtt  heart beat
int iotex_mqtt_heart_beat(struct mqtt_client *client, enum mqtt_qos qos);

#endif
