#include <device.h>
#include <string.h>
#include <drivers/flash.h>
#include <storage/flash_map.h>
#include <fs/nvs.h>
#include "local_storage.h"

static struct nvs_fs __local_storage;


/*
** @brief: Initialize local storage(zephyr nvs subsystem
** #CONFIG_NVS_LOCAL_STORAGE_SECTOR_COUNT define how many pages used as local storage
*/
int iotex_local_storage_init(void) {
    int ret;  
    struct flash_pages_info info;
    // v1.3.0-sdk-update  
    //__local_storage.offset = DT_FLASH_AREA_STORAGE_OFFSET;
    __local_storage.offset = FLASH_AREA_OFFSET(storage);


    /* Get flash page info */ 
    //  v1.3.0-sdk-update  
    //if ((ret = flash_get_page_info_by_offs(device_get_binding(DT_FLASH_DEV_NAME), __local_storage.offset, &info))) {
    if ((ret = flash_get_page_info_by_offs(device_get_binding(DT_CHOSEN_ZEPHYR_FLASH_CONTROLLER_LABEL), __local_storage.offset, &info))) {
        printk("Get flash page info failed: %d\n", ret);
        return -1;
    }

    /* Set nvs sector size as flash page size */
    __local_storage.sector_size = info.size;
    __local_storage.sector_count = CONFIG_NVS_LOCAL_STORAGE_SECTOR_COUNT;  

    /* Init nvs filesystem */
    if ((ret = nvs_init(&__local_storage, DT_CHOSEN_ZEPHYR_FLASH_CONTROLLER_LABEL))) {
        printk("Init local storage failed: %d\n", ret);
        return -1;
    }

    /* Print local storage fress space */
    printk("Local storage sector count: %u, freespace in local storage: %u, page size: %u\n",
           CONFIG_NVS_LOCAL_STORAGE_SECTOR_COUNT, nvs_calc_free_space(&__local_storage), info.size);

    return  0;
}

/*
** @breif: Clear local storage all data will gone
*/
int iotex_local_storage_clear(void) {
    return nvs_clear(&__local_storage);
}

/*
** @brief: Save data to nvs
** #id: data id in nvs, if nothing first time will auto created
** #data: data to write
** #len: data length in byte
** $return: success return 0, failed return negative code
*/
int iotex_local_storage_del(iotex_storage_id id) {
    return nvs_delete(&__local_storage, id) ? -1 : 0;
}

int iotex_local_storage_save(iotex_storage_id id, const void *data, size_t len) {
    return nvs_write(&__local_storage, id, data, len) == len ? 0 : -1;
}

/* Load always read the latest data, same as iotex_local_storage_hist cnt == 0 */
int iotex_local_storage_load(iotex_storage_id id, void *data, size_t len) {
    return nvs_read(&__local_storage, id, data, len) >= len ? 0 : -1;
}

/* History is LIFO zero always indicate the latest one, counter - 1 indicate the first one  */
int iotex_local_storage_hist(iotex_storage_id id, void *data, size_t len, uint16_t cnt) {
    return nvs_read_hist(&__local_storage, id, data, len, cnt) < 0 ? -1 : 0;
}

/* Read all data to #data point buffer, size indicate buffer size return how many items it read */
int iotex_local_storage_readall(iotex_storage_id id, void *data, size_t size, size_t item_len) {

    void *current = data;
    uint16_t read_cnt = 0;

    /* Read all data to buffer, the latest data comes first */
    for (read_cnt = 0; current - data + item_len <= size; read_cnt++, current += item_len) {

        /* No more data */
        if (iotex_local_storage_hist(id, current, item_len, read_cnt)) {
            break;
        }
    }

    return read_cnt;
}
