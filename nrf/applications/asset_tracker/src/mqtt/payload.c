#include <stdio.h>
#include <net/cloud.h>
#include "cJSON.h"
#include "cJSON_os.h"
#include "mqtt.h"
#include "config.h"
#include "hal/hal_adc.h"
#include "nvs/local_storage.h"
#include "modem/modem_helper.h"
#include "bme/bme680_helper.h"
#include "icm/icm42605_helper.h"
#include "gps_controller.h"
#include "light_sensor/tsl2572.h"

extern double latitude;
extern double longitude;
static int json_add_obj(cJSON *parent, const char *str, cJSON *item) {
    cJSON_AddItemToObject(parent, str, item);
    return 0;
}

static int json_add_str(cJSON *parent, const char *str, const char *item) {

    cJSON *json_str;
    json_str = cJSON_CreateString(item);

    if (json_str == NULL) {
        return -ENOMEM;
    }

    return json_add_obj(parent, str, json_str);
}


int iotex_mqtt_get_devinfo_payload(struct mqtt_payload *output) {

    int ret = 0;
    cJSON *root_obj = cJSON_CreateObject();
    cJSON *vbat_obj = cJSON_CreateNumber(iotex_modem_get_battery_voltage());
    cJSON *snr_obj = cJSON_CreateNumber(iotex_model_get_signal_quality());

    if (!root_obj || !vbat_obj || !snr_obj) {
        goto out;
    }

    /* Format device info */
    ret += json_add_str(root_obj, "Device", iotex_mqtt_get_client_id());
    ret += json_add_str(root_obj, "timestamp", iotex_modem_get_clock(NULL));
    ret += json_add_obj(root_obj, "VBAT", vbat_obj);
    ret += json_add_obj(root_obj, "SNR", snr_obj);

    if (ret != 0) {
        goto out;
    }

    output->buf = cJSON_PrintUnformatted(root_obj);
    output->len = strlen(output->buf);
    cJSON_Delete(root_obj);

    return 0;

out:
    cJSON_Delete(root_obj);
    return -ENOMEM;
}

int iotex_mqtt_get_env_sensor_payload(struct mqtt_payload *output) {

    iotex_storage_bme680 data;

    if (iotex_bme680_get_sensor_data(&data)) {
        return -1;
    }

    int ret = 0;
    const char *device = "BME680";
    cJSON *root_obj = cJSON_CreateObject();
    cJSON *humidity = cJSON_CreateNumber(data.humidity);
    cJSON *pressure = cJSON_CreateNumber(data.pressure);
    cJSON *temperature = cJSON_CreateNumber(data.temperature);
    cJSON *gas_resistance = cJSON_CreateNumber(data.gas_resistance);

    if (!root_obj || !humidity || !pressure || !temperature || !gas_resistance) {
        goto out;
    }

    ret += json_add_str(root_obj, "Device", device);
    ret += json_add_obj(root_obj, "humidity", humidity);
    ret += json_add_obj(root_obj, "pressure", pressure);
    ret += json_add_obj(root_obj, "temperature", temperature);
    ret += json_add_obj(root_obj, "gas_resistance", gas_resistance);
    ret += json_add_str(root_obj, "timestamp", iotex_modem_get_clock(NULL));

    if (ret != 0) {
        goto out;
    }

    output->buf = cJSON_PrintUnformatted(root_obj);
    output->len = strlen(output->buf);
    cJSON_Delete(root_obj);
    return 0;

out:
    cJSON_Delete(root_obj);
    return -ENOMEM;
}

int iotex_mqtt_get_action_sensor_payload(struct mqtt_payload *output) {

    int i;
    int ret = 0;
    const char *device = "ICM42605";

    iotex_storage_icm42605 data;
    int gyroscope[ARRAY_SIZE(data.gyroscope)];
    int accelerometer[ARRAY_SIZE(data.accelerometer)];
    if (iotex_icm42605_get_sensor_data(&data)) {
        return -1;
    }

    for (i = 0; i < ARRAY_SIZE(data.gyroscope); i++) {
        gyroscope[i] = data.gyroscope[i];
    }

    for (i = 0; i < ARRAY_SIZE(data.accelerometer); i++) {
        accelerometer[i] = data.accelerometer[i];
    }

    cJSON *root_obj = cJSON_CreateObject();
    cJSON *temperature_obj = cJSON_CreateNumber(data.temperature);
    cJSON *gyroscope_obj = cJSON_CreateIntArray(gyroscope, ARRAY_SIZE(gyroscope));
    cJSON *accelerometer_obj = cJSON_CreateIntArray(accelerometer, ARRAY_SIZE(accelerometer));

    if (!root_obj || !temperature_obj || !gyroscope_obj || !accelerometer_obj) {
        goto out;
    }

    ret += json_add_str(root_obj, "Device", device);
    ret += json_add_obj(root_obj, "gyroscope", gyroscope_obj);
    ret += json_add_obj(root_obj, "temperature", temperature_obj);
    ret += json_add_obj(root_obj, "accelerometer", accelerometer_obj);
    ret += json_add_str(root_obj, "timestamp", iotex_modem_get_clock(NULL));

    if (ret != 0) {
        goto out;
    }

    output->buf = cJSON_PrintUnformatted(root_obj);
    output->len = strlen(output->buf);
    cJSON_Delete(root_obj);
    return 0;

out:
    cJSON_Delete(root_obj);
    return -ENOMEM;

    return 0;
}

bool iotex_mqtt_sampling_data_and_store(uint16_t channel) {
    uint8_t buffer[128];
    uint32_t write_cnt = 0;
    iotex_storage_bme680 env_sensor;
    iotex_storage_icm42605 action_sensor;
	double  timestamp;
    float AmbientLight;
    uint16_t vol_Integer;

    /* Check current sampling count */
    if (!iotex_mqtt_is_need_sampling()) {
        return true;
    }

    /* Sampling data */
    if (DATA_CHANNEL_ENV_SENSOR & channel) {
        if (iotex_bme680_get_sensor_data(&env_sensor)) {
            return false;
        }
    }

    if (DATA_CHANNEL_ACTION_SENSOR & channel) {
        if (iotex_icm42605_get_sensor_data(&action_sensor)) {
            return false;
        }
    }

    /* Pack data to a buffer then store to nvs, unpack with the same sequence when uploaded */

    /* Snr */
    if (IOTEX_DATA_CHANNEL_IS_SET(channel, DATA_CHANNEL_SNR)) {
        buffer[write_cnt++] = iotex_model_get_signal_quality();
    }

    /* Vbatx100 */
    if (IOTEX_DATA_CHANNEL_IS_SET(channel, DATA_CHANNEL_VBAT)) {
        vol_Integer = (uint16_t)(iotex_modem_get_battery_voltage() * 1000);
        buffer[write_cnt++] = (uint8_t)(vol_Integer&0x00FF);
        buffer[write_cnt++] = (uint8_t)((vol_Integer>>8)&0x00FF);
    }

    /* TODO GPS */
    if (IOTEX_DATA_CHANNEL_IS_SET(channel, DATA_CHANNEL_GPS)) {
        int i = getGPS(&latitude,&longitude); 
        char *pDat = &latitude; 
        if(!i){
            for(i=0; i < sizeof(latitude); i++)
            {
                buffer[write_cnt++] = *pDat++;
            }
            pDat = &longitude;
            for(i=0; i < sizeof(longitude); i++)
            {
                buffer[write_cnt++] = *pDat++;
            }            
        }
        else{
            for(i=0; i < sizeof(latitude); i++)
            {
                buffer[write_cnt++] = 0xFF;
                buffer[write_cnt++] = 0xFF;
            }           
        }  
    }

    /* Env sensor gas */
    if (IOTEX_DATA_CHANNEL_IS_SET(channel, DATA_CHANNEL_GAS)) {
        memcpy(buffer + write_cnt, &env_sensor.gas_resistance, sizeof(env_sensor.gas_resistance));
        write_cnt += sizeof(env_sensor.gas_resistance);
    }

    /* Env sensor temperature */
    if (IOTEX_DATA_CHANNEL_IS_SET(channel, DATA_CHANNEL_TEMP)) {
        memcpy(buffer + write_cnt, &env_sensor.temperature, sizeof(env_sensor.temperature));
        write_cnt += sizeof(env_sensor.temperature);
    }

    /* Env sensor pressure */
    if (IOTEX_DATA_CHANNEL_IS_SET(channel, DATA_CHANNEL_PRESSURE)) {
        memcpy(buffer + write_cnt, &env_sensor.pressure, sizeof(env_sensor.pressure));
        write_cnt += sizeof(env_sensor.pressure);
    }

    /* Env sensor humidity */
    if (IOTEX_DATA_CHANNEL_IS_SET(channel, DATA_CHANNEL_HUMIDITY)) {
        memcpy(buffer + write_cnt, &env_sensor.humidity, sizeof(env_sensor.humidity));
        write_cnt += sizeof(env_sensor.humidity);
    }
    /* Env sensor light */
    if (IOTEX_DATA_CHANNEL_IS_SET(channel, DATA_CHANNEL_LIGHT_SENSOR)) {
        AmbientLight=iotex_Tsl2572ReadAmbientLight();
        memcpy(buffer + write_cnt, &AmbientLight, sizeof(AmbientLight));
        write_cnt += sizeof(AmbientLight);
    }  
    /* Action sensor temperature */
    if (IOTEX_DATA_CHANNEL_IS_SET(channel, DATA_CHANNEL_TEMP2)) {
        memcpy(buffer + write_cnt, &action_sensor.temperature, sizeof(action_sensor.temperature));
        write_cnt += sizeof(action_sensor.temperature);
    }

    /* Action sensor gyroscope data */
    if (IOTEX_DATA_CHANNEL_IS_SET(channel, DATA_CHANNEL_GYROSCOPE)) {
        memcpy(buffer + write_cnt, action_sensor.gyroscope, sizeof(action_sensor.gyroscope));
        write_cnt += sizeof(action_sensor.gyroscope);
    }

    /* Action sensor accelerometer */
    if (IOTEX_DATA_CHANNEL_IS_SET(channel, DATA_CHANNEL_ACCELEROMETER)) {
        memcpy(buffer + write_cnt, action_sensor.accelerometer, sizeof(action_sensor.accelerometer));
        write_cnt += sizeof(action_sensor.accelerometer);
    }

	// save timestamp
	timestamp = iotex_modem_get_clock_raw(NULL);	
	memcpy(buffer + write_cnt, &timestamp, sizeof(timestamp));
	write_cnt += sizeof(timestamp);
	//printk("write_cnt:%d\n", write_cnt);
	
	// save bytes of onece write
	set_block_size(write_cnt);	

    /* Save sampling data to nvs */
    return iotex_local_storage_save(SID_MQTT_BULK_UPLOAD_DATA, buffer, write_cnt) == 0;
}

int iotex_mqtt_get_selected_payload(uint16_t channel, struct mqtt_payload *output) {
    int i;
    iotex_storage_bme680 env_sensor;
    iotex_storage_icm42605 action_sensor;
    char esdaSign[65];
    char jsStr[130];
    int  sinLen;
    float AmbientLight=0.0;
    char random[17];
    cJSON *root_obj = cJSON_CreateObject();
    cJSON * msg_obj = cJSON_CreateObject();
    cJSON * sign_obj = cJSON_CreateObject();
    if (!root_obj||!msg_obj||!sign_obj) {
        goto out;
    }
    cJSON_AddItemToObject(root_obj, "message", msg_obj); 
    cJSON_AddItemToObject(root_obj, "signature", sign_obj);    
    if (DATA_CHANNEL_ENV_SENSOR & channel) {
        if (iotex_bme680_get_sensor_data(&env_sensor)) {
             goto out;
        }
    }

    if (DATA_CHANNEL_ACTION_SENSOR & channel) {
        if (iotex_icm42605_get_sensor_data(&action_sensor)) {
             goto out;
        }
    }

    /* Snr */
    if (IOTEX_DATA_CHANNEL_IS_SET(channel, DATA_CHANNEL_SNR)) {
        cJSON *snr = cJSON_CreateNumber(iotex_model_get_signal_quality());

        if (!snr || json_add_obj(msg_obj, "SNR", snr)) {
            goto out;
        }
    }

    /* Vbat */
    if (IOTEX_DATA_CHANNEL_IS_SET(channel, DATA_CHANNEL_VBAT)) {
        cJSON *vbat = cJSON_CreateNumber(iotex_modem_get_battery_voltage());

        if (!vbat || json_add_obj(msg_obj, "VBAT", vbat)) {
            goto out;
        }
    }
    /* TODO GPS */
    if (IOTEX_DATA_CHANNEL_IS_SET(channel, DATA_CHANNEL_GPS)) {        
        int i = getGPS(&latitude,&longitude);     
        if(!i){
            cJSON *lat = cJSON_CreateNumber(latitude);
            if (!lat || json_add_obj(msg_obj, "latitude", lat)) {
                goto out;
            }    
            cJSON *lon = cJSON_CreateNumber(longitude);
            if (!lon || json_add_obj(msg_obj, "longitude", lon)) {
                goto out;
            } 
        }
        else{
            cJSON *lat = cJSON_CreateString("null");
            if (!lat || json_add_obj(msg_obj, "latitude", lat)) {
                goto out;
            }    
            cJSON *lon = cJSON_CreateString("null");
            if (!lon || json_add_obj(msg_obj, "longitude", lon)) {
                goto out;
            }             
        }                    
    }

    /* Env sensor gas */
    if (IOTEX_DATA_CHANNEL_IS_SET(channel, DATA_CHANNEL_GAS)) {
        cJSON *gas_resistance = cJSON_CreateNumber(env_sensor.gas_resistance);

        if (!gas_resistance || json_add_obj(msg_obj, "gas_resistance", gas_resistance)) {
            goto out;
        }
    }

    /* Env sensor temperature */
    if (IOTEX_DATA_CHANNEL_IS_SET(channel, DATA_CHANNEL_TEMP)) {
        cJSON *temperature = cJSON_CreateNumber(env_sensor.temperature);

        if (!temperature || json_add_obj(msg_obj, "temperature", temperature)) {
            goto out;
        }
    }

    /* Env sensor pressure */
    if (IOTEX_DATA_CHANNEL_IS_SET(channel, DATA_CHANNEL_PRESSURE)) {
        cJSON *pressure = cJSON_CreateNumber(env_sensor.pressure);

        if (!pressure || json_add_obj(msg_obj, "pressure", pressure)) {
            goto out;
        }

    }

    /* Env sensor humidity */
    if (IOTEX_DATA_CHANNEL_IS_SET(channel, DATA_CHANNEL_HUMIDITY)) {
        cJSON *humidity = cJSON_CreateNumber(env_sensor.humidity);

        if (!humidity || json_add_obj(msg_obj, "humidity", humidity)) {
            goto out;
        }
    }

    /* Env sensor light */
    if (IOTEX_DATA_CHANNEL_IS_SET(channel, DATA_CHANNEL_LIGHT_SENSOR)) {
        AmbientLight=iotex_Tsl2572ReadAmbientLight();      
        cJSON *light = cJSON_CreateNumber(AmbientLight);

        if (!light || json_add_obj(msg_obj, "light", light)) {
            goto out;
        }
    }    

    /* Action sensor temperature */
    if (IOTEX_DATA_CHANNEL_IS_SET(channel, DATA_CHANNEL_TEMP2)) {
        cJSON *temperature = cJSON_CreateNumber(action_sensor.temperature);

        if (!temperature || json_add_obj(msg_obj, "temperature2", temperature)) {
            goto out;
        }
    }

    /* Action sensor gyroscope data */
    if (IOTEX_DATA_CHANNEL_IS_SET(channel, DATA_CHANNEL_GYROSCOPE)) {
        int gyroscope[ARRAY_SIZE(action_sensor.gyroscope)];

        for (i = 0; i < ARRAY_SIZE(action_sensor.gyroscope); i++) {
            gyroscope[i] = action_sensor.gyroscope[i];
        }

        cJSON *gyroscope_obj = cJSON_CreateIntArray(gyroscope, ARRAY_SIZE(gyroscope));

        if (!gyroscope_obj || json_add_obj(msg_obj, "gyroscope", gyroscope_obj)) {
            goto out;
        }
    }

    /* Action sensor accelerometer */
    if (IOTEX_DATA_CHANNEL_IS_SET(channel, DATA_CHANNEL_ACCELEROMETER)) {
        int accelerometer[ARRAY_SIZE(action_sensor.accelerometer)];

        for (i = 0; i < ARRAY_SIZE(action_sensor.accelerometer); i++) {
            accelerometer[i] = action_sensor.accelerometer[i];
        }

        cJSON *accelerometer_obj = cJSON_CreateIntArray(accelerometer, ARRAY_SIZE(accelerometer));

        if (!accelerometer_obj || json_add_obj(msg_obj, "accelerometer", accelerometer_obj)) {
            goto out;
        }
    }

    /* Add timestamp */
    if (json_add_str(msg_obj, "timestamp", iotex_modem_get_clock(NULL))) {
        goto out;
    }
    // get random number
    GenRandom(random);
    random[sizeof(random)-1] = 0;
    cJSON *random_obj = cJSON_CreateString(random); 
    if(!random_obj  || json_add_obj(msg_obj, "random", random_obj))
    {
        goto out;
    }
    // ecc public key
    if(get_ecc_public_key(NULL))
    {
        get_ecc_public_key(jsStr);
        jsStr[128] = 0;
        cJSON *ecc_pub_obj = cJSON_CreateString(jsStr);
        if(!ecc_pub_obj  || json_add_obj(msg_obj, "ecc_pubkey", ecc_pub_obj))
        {
            goto out;
        }        
    }
    
    output->buf = cJSON_PrintUnformatted(msg_obj);   
    doESDA_sep256r_Sign(output->buf,strlen(output->buf),esdaSign,&sinLen);   
    hex2str(esdaSign, sinLen,jsStr);
    cJSON_free(output->buf);
    memcpy(esdaSign,jsStr,64);
    esdaSign[64] = 0;    
    cJSON *esdaSign_r_Obj = cJSON_CreateString(esdaSign);
    if(!esdaSign_r_Obj  || json_add_obj(sign_obj, "r", esdaSign_r_Obj))
        goto out;
    cJSON *esdaSign_s_Obj = cJSON_CreateString(jsStr+64);
    if(!esdaSign_s_Obj  || json_add_obj(sign_obj, "s", esdaSign_s_Obj))
        goto out; 
                
    //memset(output->buf, 0, strlen(output->buf));   
    output->buf = cJSON_PrintUnformatted(root_obj);    
    output->len = strlen(output->buf);
    cJSON_Delete(root_obj);   
    return 0;

out:
    cJSON_Delete(root_obj);
    return -ENOMEM;
}


int iotex_mqtt_bin_to_json(uint8_t *buffer, uint16_t channel, struct mqtt_payload *output)
{	
	char epoch_buf[TIMESTAMP_STR_LEN];
    uint32_t write_cnt = 0;
	double  timestamp;
	float  tmp;
	int i;
    char esdaSign[65];
    char jsStr[130];
    int  sinLen;
    char random[17];
    uint16_t vol_Integer;
    cJSON *root_obj = cJSON_CreateObject();
    cJSON * msg_obj = cJSON_CreateObject();
    cJSON * sign_obj = cJSON_CreateObject();
    if (!root_obj||!msg_obj||!sign_obj) {
        goto out;
    }
	
    if (IOTEX_DATA_CHANNEL_IS_SET(channel, DATA_CHANNEL_SNR)) {
        cJSON *snr = cJSON_CreateNumber(buffer[write_cnt++]);

        if (!snr || json_add_obj(msg_obj, "SNR", snr)) {
            goto out;
        }         
    }

    /* Vbatx100 */
    if (IOTEX_DATA_CHANNEL_IS_SET(channel, DATA_CHANNEL_VBAT)) {
        vol_Integer = (uint16_t)(buffer[write_cnt++] | (buffer[write_cnt++]<<8));
        cJSON *vbat = cJSON_CreateNumber((double)(vol_Integer/1000.0));
        if (!vbat || json_add_obj(msg_obj, "VBAT", vbat)) {
            goto out;
        }
    }

    /* TODO GPS */
    if (IOTEX_DATA_CHANNEL_IS_SET(channel, DATA_CHANNEL_GPS)) {
        char *pDat =   &latitude;
        int i;
        char j;
        for(i = 0; i < sizeof(latitude);i++)
        {
            *pDat++ = buffer[write_cnt++];            
        }
        pDat =   &longitude;
        for(i = 0,j = 0xFF; i < sizeof(longitude);i++)
        {
            *pDat = buffer[write_cnt++]; 
            j &= *pDat;
            pDat++;
        }                
        if(j != 0xFF){
            cJSON *lat = cJSON_CreateNumber(latitude);
            if (!lat || json_add_obj(msg_obj, "latitude", lat)) {
                goto out;
            }    
            cJSON *lon = cJSON_CreateNumber(longitude);
            if (!lon || json_add_obj(msg_obj, "longitude", lon)) {
                goto out;
            } 
        }
        else{
            cJSON *lat = cJSON_CreateString("null");
            if (!lat || json_add_obj(msg_obj, "latitude", lat)) {
                goto out;
            }    
            cJSON *lon = cJSON_CreateString("null");
            if (!lon || json_add_obj(msg_obj, "longitude", lon)) {
                goto out;
            }             
        }  
    }

    /* Env sensor gas */
    if (IOTEX_DATA_CHANNEL_IS_SET(channel, DATA_CHANNEL_GAS)) {
		memcpy(&tmp, buffer + write_cnt, sizeof(tmp));
		write_cnt += sizeof(tmp);
        cJSON *gas_resistance = cJSON_CreateNumber(tmp);
        if (!gas_resistance || json_add_obj(msg_obj, "gas_resistance", gas_resistance)) {
            goto out;
        }	   
    }

    /* Env sensor temperature */
    if (IOTEX_DATA_CHANNEL_IS_SET(channel, DATA_CHANNEL_TEMP)) {
        memcpy(&tmp, buffer + write_cnt, sizeof(tmp));
        write_cnt += sizeof(tmp);		
        cJSON *temperature = cJSON_CreateNumber(tmp);
        if (!temperature || json_add_obj(msg_obj, "temperature", temperature)) {
            goto out;
        }		

    }

    /* Env sensor pressure */
    if (IOTEX_DATA_CHANNEL_IS_SET(channel, DATA_CHANNEL_PRESSURE)) {
        memcpy(&tmp, buffer + write_cnt, sizeof(tmp));
        write_cnt += sizeof(tmp);		
        cJSON *pressure = cJSON_CreateNumber(tmp);
        if (!pressure || json_add_obj(msg_obj, "pressure", pressure)) {
            goto out;
        }		
    }

    /* Env sensor humidity */
    if (IOTEX_DATA_CHANNEL_IS_SET(channel, DATA_CHANNEL_HUMIDITY)) {
        memcpy(&tmp, buffer + write_cnt, sizeof(tmp));
        write_cnt += sizeof(tmp);		
        cJSON *humidity = cJSON_CreateNumber(tmp);
        if (!humidity || json_add_obj(msg_obj, "humidity", humidity)) {
            goto out;
        }		
    }

    /* Env sensor light */
    if (IOTEX_DATA_CHANNEL_IS_SET(channel, DATA_CHANNEL_HUMIDITY)) {
        memcpy(&tmp, buffer + write_cnt, sizeof(tmp));
        write_cnt += sizeof(tmp);		
        cJSON *light = cJSON_CreateNumber(tmp);
        if (!light || json_add_obj(msg_obj, "light", light)) {
            goto out;
        }        
    }    

    /* Action sensor temperature */
    if (IOTEX_DATA_CHANNEL_IS_SET(channel, DATA_CHANNEL_TEMP2)) {
        memcpy(&tmp, buffer + write_cnt, sizeof(tmp));
        write_cnt += sizeof(tmp);		
        cJSON *temperature = cJSON_CreateNumber(tmp);
        if (!temperature || json_add_obj(msg_obj, "temperature2", temperature)) {
            goto out;
        }		
    }

    /* Action sensor gyroscope data */
    if (IOTEX_DATA_CHANNEL_IS_SET(channel, DATA_CHANNEL_GYROSCOPE)) {
        int gyroscope[3];
		int16_t buf[3];
		memcpy(buf, buffer + write_cnt, sizeof(buf));
		write_cnt += sizeof(buf);
        for (i = 0; i < ARRAY_SIZE(gyroscope); i++) {
            gyroscope[i] = buf[i];
        }
        cJSON *gyroscope_obj = cJSON_CreateIntArray(gyroscope, ARRAY_SIZE(gyroscope));
        if (!gyroscope_obj || json_add_obj(msg_obj, "gyroscope", gyroscope_obj)) {
            goto out;
        }
    }

    /* Action sensor accelerometer */
    if (IOTEX_DATA_CHANNEL_IS_SET(channel, DATA_CHANNEL_ACCELEROMETER)) {
        int accelerometer[3];
		int16_t buf[3];
		memcpy(buf, buffer + write_cnt, sizeof(buf));
		write_cnt += sizeof(buf);		
        for (i = 0; i < ARRAY_SIZE(accelerometer); i++) {
            accelerometer[i] = buf[i];
        }
        cJSON *accelerometer_obj = cJSON_CreateIntArray(accelerometer, ARRAY_SIZE(accelerometer));
        if (!accelerometer_obj || json_add_obj(msg_obj, "accelerometer", accelerometer_obj)) {
            goto out;
        }
    }
    /* Add timestamp */
	memcpy(&timestamp, buffer + write_cnt, sizeof(timestamp));
	//snprintf(epoch_buf, sizeof(epoch_buf), "%.0f", timestamp);
	//printk("UTC epoch %s, write_cnt:%d\n", epoch_buf, write_cnt+sizeof(timestamp));
    if (json_add_str(msg_obj, "timestamp", epoch_buf)) {
        goto out;
    }
    // get random number
    GenRandom(random);
    random[sizeof(random)-1] = 0;
    cJSON *random_obj = cJSON_CreateString(random); 
    if(!random_obj  || json_add_obj(msg_obj, "random", random_obj))
    {
        goto out;
    }

    cJSON_AddItemToObject(root_obj, "message", msg_obj);
    output->buf = cJSON_PrintUnformatted(msg_obj);
    doESDA_sep256r_Sign(output->buf,strlen(output->buf),esdaSign,&sinLen);
    hex2str(esdaSign, sinLen,jsStr);
    free(output->buf);
    memcpy(esdaSign, jsStr, 64);
    esdaSign[64]= 0;
    cJSON *esdaSign_r_Obj = cJSON_CreateString(esdaSign); 
    if(!esdaSign_r_Obj  || json_add_obj(sign_obj, "r", esdaSign_r_Obj))
        goto out;
    cJSON *esdaSign_s_Obj = cJSON_CreateString(jsStr+64); 
    if(!esdaSign_s_Obj  || json_add_obj(sign_obj, "s", esdaSign_s_Obj))
        goto out;        
    cJSON_AddItemToObject(root_obj, "signature", sign_obj);
    //memset(output->buf, 0, strlen(output->buf));        
    output->buf = cJSON_PrintUnformatted(root_obj);
    output->len = strlen(output->buf);
    cJSON_Delete(root_obj);

    return 0;

out:
    cJSON_Delete(root_obj);
    return -ENOMEM;	
}
