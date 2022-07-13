#include <zephyr.h>
#include <stdio.h>
#include <stdlib.h>
#include <drivers/i2c.h>
#include "Icm426xxDefs.h"
#include "Icm426xxTransport.h"
#include "Icm426xxDriver_HL.h"
#include "icm42605_helper.h"
#include "modem/modem_helper.h"
#include "nvs/local_storage.h"
#include "icm426xxActionDetect.h"


static struct device *__i2c_dev_icm42605;
static struct inv_icm426xx __icm_driver;

void inv_icm426xx_sleep_us(uint32_t us) {
    k_busy_wait(us);
}

uint64_t inv_icm426xx_get_time_us(void) {
    return (SYS_CLOCK_HW_CYCLES_TO_NS64(k_cycle_get_32()) / 1000);
}

static int inv_io_hal_read_reg(struct inv_icm426xx_serif *serif, uint8_t reg, uint8_t *rbuffer, uint32_t rlen) {
    return i2c_burst_read(__i2c_dev_icm42605, I2C_ADDR_ICM42605, reg, rbuffer, rlen);
}

static int inv_io_hal_write_reg(struct inv_icm426xx_serif *serif, uint8_t reg, const uint8_t *wbuffer, uint32_t wlen) {
    uint8_t rslt = 0;

    for (uint32_t i = 0; i < wlen; i++) {
        if ((rslt =  i2c_reg_write_byte(__i2c_dev_icm42605, I2C_ADDR_ICM42605, reg + i, wbuffer[i]))) {
            return rslt;
        }
    }

    return rslt;

}

static int iotex_icm42605_configure(uint8_t is_low_noise_mode,
                                    ICM426XX_ACCEL_CONFIG0_FS_SEL_t acc_fsr_g,
                                    ICM426XX_GYRO_CONFIG0_FS_SEL_t gyr_fsr_dps,
                                    ICM426XX_ACCEL_CONFIG0_ODR_t acc_freq,
                                    ICM426XX_GYRO_CONFIG0_ODR_t gyr_freq,
                                    uint8_t is_rtc_mode)
{
    int rc = 0;
    uint32_t startup_delay;

    rc |= inv_icm426xx_enable_clkin_rtc(&__icm_driver, is_rtc_mode);

    rc |= inv_icm426xx_set_accel_fsr(&__icm_driver, acc_fsr_g);
    rc |= inv_icm426xx_set_gyro_fsr(&__icm_driver, gyr_fsr_dps);

    rc |= inv_icm426xx_set_accel_frequency(&__icm_driver, acc_freq);
    rc |= inv_icm426xx_set_gyro_frequency(&__icm_driver, gyr_freq);

    if (is_low_noise_mode)
        rc |= inv_icm426xx_enable_accel_low_noise_mode(&__icm_driver);
    else
        rc |= inv_icm426xx_enable_accel_low_power_mode(&__icm_driver);

    rc |= inv_icm426xx_enable_gyro_low_noise_mode(&__icm_driver);

    /* Wait Max of ICM426XX_GYR_STARTUP_TIME_US and ICM426XX_ACC_STARTUP_TIME_US*/
    startup_delay = (ICM426XX_GYR_STARTUP_TIME_US > ICM426XX_ACC_STARTUP_TIME_US) ? ICM426XX_GYR_STARTUP_TIME_US : ICM426XX_ACC_STARTUP_TIME_US;
    inv_icm426xx_sleep_us(startup_delay);

    return rc;
}

int  rawdataConf(void)
{
    return iotex_icm42605_configure((uint8_t)IS_LOW_NOISE_MODE,
                                     ICM426XX_ACCEL_CONFIG0_FS_SEL_4g,
                                     ICM426XX_GYRO_CONFIG0_FS_SEL_2000dps,
                                     ICM426XX_ACCEL_CONFIG0_ODR_1_KHZ,
                                     ICM426XX_GYRO_CONFIG0_ODR_1_KHZ,
                                     (uint8_t)TMST_CLKIN_32K);  
}

int  rawdataDetect(void)
{
    int status = 0;
    uint8_t int_status;    
    status |= inv_icm426xx_read_reg(&__icm_driver, MPUREG_INT_STATUS, 1, &int_status);
    if (status) {
        printk("error rawdata: %d\n",int_status);        
        return 0;
    }
    if (int_status & BIT_INT_STATUS_DRDY) {
        printk("rawdata detected \n");
    }
    return 1;
}

int iotex_icm42605_init(void)
{
    int rc = 0;
    uint8_t who_am_i;
    static struct inv_icm426xx_serif icm_serif;

    /* ICM42605 i2c bus init */
    if (!(__i2c_dev_icm42605 = device_get_binding(I2C_DEV_ICM42605))) {
        printk("I2C: Device driver[%s] not found.\n", I2C_DEV_ICM42605);
        return -1;
    }

    /* ICM42605 init */
    icm_serif.context = 0;  /* no need */
    icm_serif.read_reg = inv_io_hal_read_reg;
    icm_serif.write_reg = inv_io_hal_write_reg;
    icm_serif.max_read  = 1024 * 32;  /* maximum number of bytes allowed per serial read */
    icm_serif.max_write = 1024 * 32;  /* maximum number of bytes allowed per serial write */
    icm_serif.serif_type = SERIF_TYPE;

    rc = inv_icm426xx_init(&__icm_driver, &icm_serif, NULL); 
    rc |= inv_icm426xx_configure_fifo(&__icm_driver, INV_ICM426XX_FIFO_DISABLED);

    if (rc != INV_ERROR_SUCCESS) {
        printk("!!! ERROR : failed to initialize Icm426xx.\n");
        return rc;
    }

    /* Check WHOAMI */
    rc = inv_icm426xx_get_who_am_i(&__icm_driver, &who_am_i);

    if (rc != INV_ERROR_SUCCESS) {
        printk("!!! ERROR : failed to read Icm426xx whoami value.\n");
        return rc;
    }

    if (who_am_i != ICM_WHOAMI) {
        printk("!!! ERROR :  bad WHOAMI value. Got 0x%02x (expected: 0x%02x)\n", who_am_i, ICM_WHOAMI);
        return INV_ERROR;
    }

    printk("ICM42605 WHOAMI value. Got 0x%02x\n", who_am_i);

/*
    return  iotex_icm42605_configure((uint8_t)IS_LOW_NOISE_MODE,
                                     ICM426XX_ACCEL_CONFIG0_FS_SEL_4g,
                                     ICM426XX_GYRO_CONFIG0_FS_SEL_2000dps,
                                     ICM426XX_ACCEL_CONFIG0_ODR_1_KHZ,
                                     ICM426XX_GYRO_CONFIG0_ODR_1_KHZ,
                                     (uint8_t)TMST_CLKIN_32K);
*/
    return   icm426xxConfig(ACT_RAW);                                  
}

struct inv_icm426xx * getICMDriver(void)
{
    return &__icm_driver;
}

void inv_icm42605_format_data(const uint8_t endian, const uint8_t *in, uint16_t *out)
{
    if (endian == ICM426XX_INTF_CONFIG0_DATA_BIG_ENDIAN)
        *out = (in[0] << 8) | in[1];
    else
        *out = (in[1] << 8) | in[0];
}

int iotex_icm42605_get_sensor_data(iotex_storage_icm42605 *icm42605) {
    int i;
    int status = 0;
    uint8_t int_status;
    uint8_t temperature[2];
    uint8_t accel[ACCEL_DATA_SIZE];
    uint8_t gyro[GYRO_DATA_SIZE];
    uint16_t ftemperature;
    uint16_t faccel[ACCEL_DATA_SIZE / 2];
    uint16_t fgyro[GYRO_DATA_SIZE / 2];

    /* Ensure data ready status bit is set */
    status |= inv_icm426xx_read_reg(&__icm_driver, MPUREG_INT_STATUS, 1, &int_status);

    if (status) {
        return status;
    }

    if (int_status & BIT_INT_STATUS_DRDY) {

        status = inv_icm426xx_read_reg(&__icm_driver, MPUREG_TEMP_DATA0_UI, TEMP_DATA_SIZE, temperature);
        inv_icm42605_format_data(__icm_driver.endianess_data, temperature, (uint16_t *) & (ftemperature));

        status |= inv_icm426xx_read_reg(&__icm_driver, MPUREG_ACCEL_DATA_X0_UI, ACCEL_DATA_SIZE, accel);
        inv_icm42605_format_data(__icm_driver.endianess_data, &accel[0], (uint16_t *)&faccel[0]);
        inv_icm42605_format_data(__icm_driver.endianess_data, &accel[2], (uint16_t *)&faccel[1]);
        inv_icm42605_format_data(__icm_driver.endianess_data, &accel[4], (uint16_t *)&faccel[2]);

        status |= inv_icm426xx_read_reg(&__icm_driver, MPUREG_GYRO_DATA_X0_UI, GYRO_DATA_SIZE, gyro);
        inv_icm42605_format_data(__icm_driver.endianess_data, &gyro[0], (uint16_t *)&fgyro[0]);
        inv_icm42605_format_data(__icm_driver.endianess_data, &gyro[2], (uint16_t *)&fgyro[1]);
        inv_icm42605_format_data(__icm_driver.endianess_data, &gyro[4], (uint16_t *)&fgyro[2]);

        if ((faccel[0] != INVALID_VALUE_FIFO) && (fgyro[0] != INVALID_VALUE_FIFO)) {

            icm42605->temperature = (ftemperature / 132.48) + 25;

            for (i = 0; i < ARRAY_SIZE(icm42605->gyroscope); i++) {
                icm42605->gyroscope[i] = (int16_t)fgyro[i];
            }

            for (i = 0; i < ARRAY_SIZE(icm42605->accelerometer); i++) {
                icm42605->accelerometer[i] = (int16_t)faccel[i];
            }

        }
        else {
            return -1;
        }
    }

    /*else: Data Ready was not set*/

    return status;
}
