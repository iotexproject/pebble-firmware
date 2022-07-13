#include "mqtt.h"
#include "config.h"
#include "cJSON.h"
#include "cJSON_os.h"
#include "nvs/local_storage.h"
#include "ui.h"

/* Global configure */
static iotex_mqtt_config __config = {
    .data_channel = CONFIG_MQTT_CONFIG_DATA_CHANNEL,

#ifdef CONFIG_MQTT_CONFIG_BULK_UPLOAD
    .bulk_upload = CONFIG_MQTT_CONFIG_BULK_UPLOAD,
    .bulk_upload_sampling_cnt = CONFIG_MQTT_CONFIG_SAMPLING_COUNT,
    .bulk_upload_sampling_freq = CONFIG_MQTT_CONFIG_SAMPLING_FREQUENCY
#else   
    .upload_period = CONFIG_MQTT_CONFIG_UPLOAD_PERIOD,    
#endif
};
struct sys_mutex iotex_config_mutex;

static bool save_mqtt_config() {
    /* Delete sid, only save one piece config, do not save history */
    if (iotex_local_storage_del(SID_MQTT_DATA_CHANNEL_CONFIG)) {
        return false;
    }

    return iotex_local_storage_save(SID_MQTT_DATA_CHANNEL_CONFIG, &__config, sizeof(__config)) == 0;
}

bool iotex_mqtt_is_bulk_upload(void) {
    return __config.bulk_upload;
}

uint16_t iotex_mqtt_get_data_channel(void) {
    return __config.data_channel;
}

uint16_t iotex_mqtt_get_upload_period(void) {
    return __config.bulk_upload ? 0 : (__config.upload_period == 0 ? 30 : __config.upload_period);
}

uint16_t iotex_mqtt_get_bulk_upload_count(void) {
    return __config.bulk_upload_sampling_cnt;
}

uint16_t iotex_mqtt_get_sampling_frequency(void) {
    return __config.bulk_upload_sampling_freq;
}

uint16_t iotex_mqtt_get_current_upload_count(void) {
    return __config.current_upload_cnt;
}

uint16_t iotex_mqtt_get_current_sampling_count(void) {
    return __config.current_sampling_cnt;
}

bool iotex_mqtt_is_need_sampling(void) {
	printk("current_sampling_cnt:%d, bulk_upload_sampling_cnt:%d\n", __config.current_sampling_cnt,__config.bulk_upload_sampling_cnt);
    return __config.current_sampling_cnt < __config.bulk_upload_sampling_cnt;
}

bool iotex_mqtt_is_bulk_upload_over(void) {
	return  (__config.current_upload_cnt >= __config.bulk_upload_sampling_cnt);
}

bool iotex_mqtt_inc_current_upload_count(void) {
    /* Increase upload count */
    __config.current_upload_cnt++;

    /* Save upload count, for breakpoint retransmission */
    if (__config.current_upload_cnt >= __config.bulk_upload_sampling_cnt) {
        /* All of data is uploaded, switch sampling mode */
        __config.current_upload_cnt = 0;
        __config.current_sampling_cnt = 0;
        save_mqtt_config();
        return true;
    }

    save_mqtt_config();
    return false;
}

bool iotex_mqtt_inc_current_sampling_count(void) {
    /* Increase sampling count */
    __config.current_sampling_cnt++;

    /* Save sampling count, for breakpoint resampling */
    if (__config.current_sampling_cnt >= __config.bulk_upload_sampling_cnt) {
        __config.current_upload_cnt = 0;
        save_mqtt_config();
        return true;
    }

    save_mqtt_config();
    return false;
}

void config_mutex_lock(void)
{
    sys_mutex_lock(&iotex_config_mutex, K_FOREVER);
}
void config_mutex_unlock(void)
{
    sys_mutex_unlock(&iotex_config_mutex);
}

static void print_mqtt_config(const iotex_mqtt_config *config, const char *title) {
    printk("%s: bulk_upload: [%s], data_channel:[0x%04x], upload_period[%u], "
           "bulk_upload_sampling_cnt[%u], bulk_upload_sampling_freq[%u],"
           "current_upload_cnt[%u], current_sampling_cnt[%u]\n",
           title, config->bulk_upload ? "yes" : "no", config->data_channel, config->upload_period,
           config->bulk_upload_sampling_cnt, config->bulk_upload_sampling_freq,
           config->current_upload_cnt, config->current_sampling_cnt);
}

/*
    parse jscon update configure
    return 0
*/
static int iotex_mqtt_parse_config(const uint8_t *payload, uint32_t len, iotex_mqtt_config *config) {
    char config_buffer[CONFIG_MQTT_PAYLOAD_BUFFER_SIZE];
    uint32_t  dat;
    int ret = 1;

    if(len >= CONFIG_MQTT_PAYLOAD_BUFFER_SIZE)
    {
        printk("config size not enough len: %d, conf_size:%d\n",len, CONFIG_MQTT_PAYLOAD_BUFFER_SIZE);
        return 0;
    }

    memcpy(config_buffer, payload, len);
    config_buffer[len] = 0;

#ifdef CONFIG_DEBUG_MQTT_CONFIG
    printk("[%s]: Received mqtt json config: [%u]%s\n", __func__, len, config_buffer);
#endif

    cJSON *bulk_upload = NULL;
    cJSON *data_channel = NULL;
    cJSON *upload_period = NULL;
    cJSON *bulk_upload_sampling_cnt = NULL;
    cJSON *bulk_upload_sampling_freq = NULL;
    cJSON *serverBeep = NULL;
    cJSON *root_obj = cJSON_Parse(config_buffer);

    if (!root_obj) {
        const char *err_ptr = cJSON_GetErrorPtr();

        if (!err_ptr) {
            printk("[%s:%d] error before: %s\n", __func__, __LINE__, err_ptr);
        }
        ret = 0;
        goto out;
    }

    /* Clear config data */
    // update config one by one 
    //memset(config, 0, sizeof(iotex_mqtt_config));

    bulk_upload = cJSON_GetObjectItem(root_obj, "bulk_upload");
    data_channel = cJSON_GetObjectItem(root_obj, "data_channel");
    upload_period = cJSON_GetObjectItem(root_obj, "upload_period");
    bulk_upload_sampling_cnt = cJSON_GetObjectItem(root_obj, "bulk_upload_sampling_cnt");
    bulk_upload_sampling_freq = cJSON_GetObjectItem(root_obj, "bulk_upload_sampling_freq");
    serverBeep = cJSON_GetObjectItem(root_obj, "beep");
    
    /* Notice: 0 ==> false, otherwise true */
    if (bulk_upload && cJSON_IsString(bulk_upload)) {
        dat = config->bulk_upload;
        config->bulk_upload = atoi(bulk_upload->valuestring);
        config->bulk_upload_sampling_freq = 10;
        config->bulk_upload_sampling_cnt = 10;
        if(dat != config->bulk_upload)
            ret = 2;
        
    }

    if (data_channel && cJSON_IsString(data_channel)) {
        config->data_channel = atoi(data_channel->valuestring);
    }

    if (upload_period && cJSON_IsString(upload_period)) { 
        config->upload_period = atoi(upload_period->valuestring);
        if(!config->bulk_upload)
            RestartEnvWork(config->upload_period);
    }

    if (bulk_upload_sampling_cnt && cJSON_IsString(bulk_upload_sampling_cnt)) {
        dat = config->bulk_upload_sampling_cnt;        
        config->bulk_upload_sampling_cnt = atoi(bulk_upload_sampling_cnt->valuestring);        
        if(config->bulk_upload &&(dat != config->bulk_upload_sampling_cnt)){
                ret = 2;     
        } 
    }

    if (bulk_upload_sampling_freq && cJSON_IsString(bulk_upload_sampling_freq)) {        
        config->bulk_upload_sampling_freq = atoi(bulk_upload_sampling_freq->valuestring);
        if(config->bulk_upload)
            RestartEnvWork(config->bulk_upload_sampling_freq);
    }

    if (serverBeep && cJSON_IsString(serverBeep)) {
        onBeepMePressed(atoi(serverBeep->valuestring));        
    }       

#ifdef CONFIG_DEBUG_MQTT_CONFIG
    print_mqtt_config(config, __func__);
#endif

    cJSON_Delete(root_obj);
    return ret;

out:
    cJSON_Delete(root_obj);
    return ret;
}

/* Parse configure and save it to nvs */
void iotex_mqtt_update_config(const uint8_t *payload, uint32_t len) {
    int ret;    

    config_mutex_lock();
    if (!(ret = iotex_mqtt_parse_config(payload, len, &__config))) {    
        config_mutex_unlock();
        printk("[%s:%d]: Parse configure failed!\n", __func__, __LINE__);
        return;
    }
    /* If enable bulk upload delete local sampling data */
    if (ret == 2) {    
        __config.current_sampling_cnt = 0;
        __config.current_upload_cnt = 0;             
        iotex_local_storage_del(SID_MQTT_BULK_UPLOAD_DATA);    
        printk("Local sampling data deleted!!!\n");
    }
    /* Save new configure */
    if (!save_mqtt_config()) {
        config_mutex_unlock();
        printk("[%s:%d]: Save configure failed!\n", __func__, __LINE__); 
        return;       
    }    
    config_mutex_unlock();
}

/* Load confifure from nvs and apply */
void iotex_mqtt_load_config(void) {
    if (!iotex_local_storage_load(SID_MQTT_DATA_CHANNEL_CONFIG, &__config, sizeof(__config))) {

		/*  a quick test solution */
		//__config.bulk_upload_sampling_cnt = 2;		
        //__config.data_channel=0x1FF7;
        //printk(" !!! Debug version open all sensors");
        //printk(" __config.data_channel:0x%x\n",  __config.data_channel);
		//save_mqtt_config();

        print_mqtt_config(&__config, __func__);
    }
    sys_mutex_init(&iotex_config_mutex);
}
// bytes  of once  write 
void set_block_size(uint32_t size)
{
	__config.size_of_block = size;
}
//  get  block size
uint32_t get_block_size(void)
{
	return __config.size_of_block;
}
//  get nvs read entry
uint16_t get_his_block(void)
{
	if(__config.current_upload_cnt < __config.bulk_upload_sampling_cnt)
		return (__config.bulk_upload_sampling_cnt - __config.current_upload_cnt -1);
	else
		return 0;
}
