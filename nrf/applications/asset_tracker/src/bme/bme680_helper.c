#include <zephyr.h>
#include <stdio.h>
#include <drivers/i2c.h>
#include <device.h>
#include <string.h>
#include "bme680.h"
#include "bme680_helper.h"
#include "nvs/local_storage.h"
#include "modem/modem_helper.h"
#include "env_sensors.h"

static struct bme680_dev __gas_sensor;
static struct device *__i2c_dev_bme680;


/* Return 0 for Success, non-zero for failure */
static int8_t user_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len) {
    return i2c_burst_read(__i2c_dev_bme680, I2C_ADDR_BME680, reg_addr, reg_data, ((uint32_t)len));
}

/* Return 0 for Success, non-zero for failure */
static int8_t user_i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len) {

    uint16_t i;
    int8_t rslt = 0;

    for (i = 0; i < len; i++) {
        i2c_reg_write_byte(__i2c_dev_bme680, I2C_ADDR_BME680, reg_addr + i, *(reg_data + i));
    }

    return rslt;
}

static void user_delay_ms(uint32_t period) {
    //k_sleep(period);
    k_usleep(period*1000);
}

static int8_t user_config_bme680(void) {

    uint8_t set_required_settings;

    /* Set the temperature, pressure and humidity settings */
    __gas_sensor.tph_sett.os_hum = BME680_OS_2X;
    __gas_sensor.tph_sett.os_pres = BME680_OS_4X;
    __gas_sensor.tph_sett.os_temp = BME680_OS_8X;
    __gas_sensor.tph_sett.filter = BME680_FILTER_SIZE_3;

    /* Set the remaining gas sensor settings and link the heating profile */
    __gas_sensor.gas_sett.run_gas = BME680_ENABLE_GAS_MEAS;
    /* Create a ramp heat waveform in 3 steps */
    __gas_sensor.gas_sett.heatr_temp = 320; /* degree Celsius */
    __gas_sensor.gas_sett.heatr_dur = 150; /* milliseconds */

    /* Select the power mode */
    /* Must be set before writing the sensor configuration */
    __gas_sensor.power_mode = BME680_FORCED_MODE;

    /* Set the required sensor settings needed */
    set_required_settings = BME680_OST_SEL | BME680_OSP_SEL | BME680_OSH_SEL | BME680_FILTER_SEL | BME680_GAS_SENSOR_SEL;

    /* Set the desired sensor configuration */
    int8_t rslt = BME680_OK;
    rslt = bme680_set_sensor_settings(set_required_settings, &__gas_sensor);

    /* Set the power mode */
    rslt = bme680_set_sensor_mode(&__gas_sensor);
    return rslt;
}

int iotex_bme680_init(void) {

    /* BME680 i2c bus init */
    uint8_t chip_id = 0;
    int8_t ret = BME680_OK;

    if (!(__i2c_dev_bme680 = device_get_binding(I2C_DEV_BME680))) {
        printk("I2C: Device driver[%s] not found.\n", I2C_DEV_BME680);
        return -1;
    }

    /* Read chip id */
    if (i2c_reg_read_byte(__i2c_dev_bme680, I2C_ADDR_BME680, 0xd0, &chip_id) != 0) {
        printk("Error on i2c_read(bme680)\n");
    } else {
        printk("BMD680 ID = 0x%x\r\n", chip_id);
    }

    /* Use i2c interface comm with BME680 */
    __gas_sensor.dev_id = I2C_ADDR_BME680;
    __gas_sensor.intf = BME680_I2C_INTF;
    __gas_sensor.read = user_i2c_read;
    __gas_sensor.write = user_i2c_write;
    __gas_sensor.delay_ms = user_delay_ms;
    /* amb_temp can be set to 25 prior to configuring the gas sensor
     * or by performing a few temperature readings without operating the gas sensor.
     */
    __gas_sensor.amb_temp = 25;

    /* zero -> Success / +ve value -> Warning / -ve value -> Error */
    if ((ret = bme680_init(&__gas_sensor) < 0)) {
        printk("bme680_init init error: %d\n", ret);
        return ret;
    }

    /* Configure the sensor */
    return user_config_bme680();
}

int iotex_bme680_get_sensor_data(iotex_storage_bme680 *bme680) {

    uint16_t meas_period;
    int8_t rslt = BME680_OK;
    bme680_get_profile_dur(&meas_period, &__gas_sensor);

    struct bme680_field_data data;

    /* Delay till the measurement is ready */
    user_delay_ms(500);

    if ((rslt = bme680_get_sensor_data(&data, &__gas_sensor))) {
        return rslt;
    }

    /* Copy data to iotex_storage_bme680 */
    bme680->pressure = data.pressure / 100.0;
    bme680->humidity = data.humidity / 1000.0;
    bme680->temperature = data.temperature / 100.0;
    bme680->gas_resistance = data.gas_resistance;

    bme680->temperature -= 6;
    bme680->humidity = bme680->humidity*1.35;
    bme680->humidity = bme680->humidity > 100.00 ? 100.00 : bme680->humidity;
    /* Trigger the next measurement if you would like to read data out continuously */
    if (__gas_sensor.power_mode == BME680_FORCED_MODE) {
        rslt = bme680_set_sensor_mode(&__gas_sensor);
    }

    return rslt;
}