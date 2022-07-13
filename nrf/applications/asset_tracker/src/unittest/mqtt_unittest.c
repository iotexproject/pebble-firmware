#include "unittest.h"
#include "mqtt/mqtt.h"
#include "mqtt/config.h"

static void test_device_info_payload() {

    struct mqtt_payload device_info;
    UNITTEST_ASSERT_EQ(iotex_mqtt_get_devinfo_payload(&device_info), 0);

    printk("%s:[%d] %s\n", __func__, device_info.len, device_info.buf);
    free(device_info.buf);
    UNITTEST_AUTO_PASS();
}

static void test_env_sensor_payload() {

    struct mqtt_payload env_sonsor;
    UNITTEST_ASSERT_EQ(iotex_mqtt_get_env_sensor_payload(&env_sonsor), 0);

    printk("%s:[%d] %s\n", __func__, env_sonsor.len, env_sonsor.buf);
    free(env_sonsor.buf);
    UNITTEST_AUTO_PASS();
}

static void test_action_sensor_payload() {

    struct mqtt_payload action_sensor;
    UNITTEST_ASSERT_EQ(iotex_mqtt_get_action_sensor_payload(&action_sensor), 0);

    printk("%s:[%d] %s\n", __func__, action_sensor.len, action_sensor.buf);
    free(action_sensor.buf);
    UNITTEST_AUTO_PASS();
}

static void test_get_selected_payload() {

    uint16_t data_channel;
    struct mqtt_payload payload;

    /* Device info */
    UNITTEST_ASSERT_EQ(iotex_mqtt_get_selected_payload(DATA_CHANNEL_DEVINFO, &payload), 0);
    printk("%s:[%d] %s\n", __func__, payload.len, payload.buf);
    free(payload.buf);

    /* Env sensor */
    UNITTEST_ASSERT_EQ(iotex_mqtt_get_selected_payload(DATA_CHANNEL_ENV_SENSOR, &payload), 0);
    printk("%s:[%d] %s\n", __func__, payload.len, payload.buf);
    free(payload.buf);

    /* Action sensor */
    UNITTEST_ASSERT_EQ(iotex_mqtt_get_selected_payload(DATA_CHANNEL_ACTION_SENSOR, &payload), 0);
    printk("%s:[%d] %s\n", __func__, payload.len, payload.buf);
    free(payload.buf);

    data_channel = DATA_CHANNEL_DEVINFO | DATA_CHANNEL_TEMP | DATA_CHANNEL_TEMP2 | DATA_CHANNEL_GYROSCOPE;
    UNITTEST_ASSERT_EQ(iotex_mqtt_get_selected_payload(data_channel, &payload), 0);
    printk("%s:[%d] %s\n", __func__, payload.len, payload.buf);
    free(payload.buf);

    UNITTEST_AUTO_PASS();
}

static void test_parse_json_config() {

    iotex_mqtt_config config;
    const char *json_str1 = "{"
                            "\"bulk_upload\": 1,"
                            "\"data_channel\": 2,"
                            "\"upload_period\": 3,"
                            "\"bulk_upload_sampling_cnt\": 4,"
                            "\"bulk_upload_sampling_freq\": 5"
                            "}";

    UNITTEST_ASSERT_EQ(iotex_mqtt_parse_config(json_str1, strlen(json_str1), &config), true);

    UNITTEST_ASSERT_EQ(config.bulk_upload, true);
    UNITTEST_ASSERT_EQ(config.data_channel, 2);
    UNITTEST_ASSERT_EQ(config.upload_period, 3);
    UNITTEST_ASSERT_EQ(config.bulk_upload_sampling_cnt, 4);
    UNITTEST_ASSERT_EQ(config.bulk_upload_sampling_freq, 5);
    UNITTEST_ASSERT_EQ(config.current_sampling_cnt, 0);
    UNITTEST_ASSERT_EQ(config.current_upload_cnt, 0);

    config.current_upload_cnt = 100;
    config.current_sampling_cnt = 10;

    const char *json_str2 = "{"
                            "\"bulk_upload\": 0,"
                            "\"data_channel\": 22,"
                            "\"upload_period\": 23,"
                            "\"bulk_upload_sampling_cnt\": 24,"
                            "\"bulk_upload_sampling_freq\": 25"
                            "}";

    UNITTEST_ASSERT_EQ(iotex_mqtt_parse_config(json_str2, strlen(json_str2), &config), true);

    UNITTEST_ASSERT_EQ(config.bulk_upload, false);
    UNITTEST_ASSERT_EQ(config.data_channel, 22);
    UNITTEST_ASSERT_EQ(config.upload_period, 23);
    UNITTEST_ASSERT_EQ(config.bulk_upload_sampling_cnt, 24);
    UNITTEST_ASSERT_EQ(config.bulk_upload_sampling_freq, 25);
    UNITTEST_ASSERT_EQ(config.current_sampling_cnt, 0);
    UNITTEST_ASSERT_EQ(config.current_upload_cnt, 0);

    UNITTEST_AUTO_PASS();
}

static void test_data_channel() {

    uint16_t data_channel = 0x7;

    UNITTEST_ASSERT_EQ(IOTEX_DATA_CHANNEL_IS_SET(data_channel, DATA_CHANNEL_GPS), true);
    UNITTEST_ASSERT_EQ(IOTEX_DATA_CHANNEL_IS_SET(data_channel, DATA_CHANNEL_SNR), true);
    UNITTEST_ASSERT_EQ(IOTEX_DATA_CHANNEL_IS_SET(data_channel, DATA_CHANNEL_VBAT), true);
    UNITTEST_ASSERT_EQ(IOTEX_DATA_CHANNEL_IS_SET(data_channel, DATA_CHANNEL_TEMP), false);

    data_channel = DATA_CHANNEL_ENV_SENSOR;
    UNITTEST_ASSERT_EQ(IOTEX_DATA_CHANNEL_IS_SET(data_channel, DATA_CHANNEL_GAS), true);
    UNITTEST_ASSERT_EQ(IOTEX_DATA_CHANNEL_IS_SET(data_channel, DATA_CHANNEL_TEMP), true);
    UNITTEST_ASSERT_EQ(IOTEX_DATA_CHANNEL_IS_SET(data_channel, DATA_CHANNEL_PRESSURE), true);
    UNITTEST_ASSERT_EQ(IOTEX_DATA_CHANNEL_IS_SET(data_channel, DATA_CHANNEL_HUMIDITY), true);
    UNITTEST_ASSERT_EQ(IOTEX_DATA_CHANNEL_IS_SET(data_channel, DATA_CHANNEL_ENV_SENSOR), true);

    data_channel = DATA_CHANNEL_ACTION_SENSOR;
    UNITTEST_ASSERT_EQ(IOTEX_DATA_CHANNEL_IS_SET(data_channel, DATA_CHANNEL_TEMP2), true);
    UNITTEST_ASSERT_EQ(IOTEX_DATA_CHANNEL_IS_SET(data_channel, DATA_CHANNEL_GYROSCOPE), true);
    UNITTEST_ASSERT_EQ(IOTEX_DATA_CHANNEL_IS_SET(data_channel, DATA_CHANNEL_ACCELEROMETER), true);
    UNITTEST_ASSERT_EQ(IOTEX_DATA_CHANNEL_IS_SET(data_channel, DATA_CHANNEL_CUSTOM_MOTION), true);
    UNITTEST_ASSERT_EQ(IOTEX_DATA_CHANNEL_IS_SET(data_channel, DATA_CHANNEL_ACTION_SENSOR), true);

    UNITTEST_AUTO_PASS();
}

static void test_get_mqtt_config() {
    UNITTEST_ASSERT_EQ(iotex_mqtt_get_data_channel(), CONFIG_MQTT_CONFIG_DATA_CHANNEL);

#ifdef CONFIG_MQTT_CONFIG_BULK_UPLOAD
    UNITTEST_ASSERT_EQ(iotex_mqtt_is_bulk_upload(), true);
    UNITTEST_ASSERT_EQ(iotex_mqtt_get_upload_period(), 0);
    UNITTEST_ASSERT_EQ(iotex_mqtt_get_bulk_upload_count(), CONFIG_MQTT_CONFIG_SAMPLING_COUNT);
    UNITTEST_ASSERT_EQ(iotex_mqtt_get_sampling_frequency(), CONFIG_MQTT_CONFIG_SAMPLING_FREQUENCY);
#else
    UNITTEST_ASSERT_EQ(iotex_mqtt_is_bulk_upload(), false);
    UNITTEST_ASSERT_EQ(iotex_mqtt_get_upload_period(), CONFIG_MQTT_CONFIG_UPLOAD_PERIOD);
    UNITTEST_ASSERT_EQ(iotex_mqtt_get_bulk_upload_count(), 0);
    UNITTEST_ASSERT_EQ(iotex_mqtt_get_sampling_frequency(), 0);
#endif

    UNITTEST_AUTO_PASS();
}

void mqtt_unittest() {

    test_device_info_payload();
    test_env_sensor_payload();
    test_action_sensor_payload();
    test_parse_json_config();
    test_data_channel();
    test_get_selected_payload();
    test_get_mqtt_config();

    UNITTEST_AUTO_PASS();
}