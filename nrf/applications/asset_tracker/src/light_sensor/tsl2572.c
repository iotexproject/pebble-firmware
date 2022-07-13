#include <zephyr.h>
#include <stdio.h>
#include <drivers/i2c.h>
#include <device.h>
#include <string.h>
#include "tsl2572.h"
#include "nvs/local_storage.h"
#include "modem/modem_helper.h"

#define max(a,b) ((a)>(b)?(a):(b))

int gain_val = 0;

static struct device *__i2c_dev_tsl2572;


/* Return 0 for Success, non-zero for failure */
static int8_t tsl2572_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint16_t len) {
    return i2c_burst_read(__i2c_dev_tsl2572, TSL2572_I2CADDR, reg_addr, reg_data, ((uint32_t)len));
}

/* Return 0 for Success, non-zero for failure */
static int8_t tsl2572_i2c_write(uint8_t reg_addr, uint8_t *reg_data, uint16_t len) {

    uint16_t i;
    int8_t rslt = 0;

    for (i = 0; i < len; i++) {
        i2c_reg_write_byte(__i2c_dev_tsl2572, TSL2572_I2CADDR, reg_addr + i, *(reg_data + i));
    }

    return rslt;
}

int iotex_TSL2572_init(uint8_t gain)
{
    uint8_t chip_id = 0;

    if (!(__i2c_dev_tsl2572 = device_get_binding(I2C_DEV_TSL2572))) {
        printk("I2C: Device driver[%s] not found.\n", I2C_DEV_TSL2572);
        return -1;
    }
    /* Read chip id */
    if (i2c_reg_read_byte(__i2c_dev_tsl2572, TSL2572_I2CADDR, 
           TSL2572_CMD_REGISTER |TSL2572_CMDS_WHOAMI, &chip_id) != 0) {
        printk("Error on i2c_read(tsl2572)\n");
        return -1;
    } else {
        printk("TSL2572 ID = 0x%x\r\n", chip_id);
    }
    //set gain
    i2c_reg_write_byte(__i2c_dev_tsl2572, TSL2572_I2CADDR, 
           TSL2572_CMD_REGISTER|TSL2572_CMDS_CONTROL, gain);
    //51.87 ms
    i2c_reg_write_byte(__i2c_dev_tsl2572, TSL2572_I2CADDR, 
           TSL2572_CMD_REGISTER |TSL2572_CMDS_ALS_TIMING, 0xED);
    //turn on
    i2c_reg_write_byte(__i2c_dev_tsl2572, TSL2572_I2CADDR, 
           TSL2572_CMD_REGISTER |TSL2572_CMDS_ENABLE, 0x03);
    if(GAIN_DIVIDE_6){
        //scale gain by 0.16
        i2c_reg_write_byte(__i2c_dev_tsl2572, TSL2572_I2CADDR, 
                 TSL2572_CMD_REGISTER |TSL2572_CMDS_CONFIG, 0x04);
    }
    if(gain==GAIN_1X) gain_val = 1;
    else if(gain==GAIN_8X) gain_val = 8;
    else if(gain==GAIN_16X) gain_val = 16;
    else if(gain==GAIN_120X) gain_val = 120;

    return 0;
}

float iotex_Tsl2572ReadAmbientLight(void)
{     
    uint8_t data[4]; 
    int c0,c1;
    float lux1,lux2,cpl;

    tsl2572_i2c_read(0xA0 | 0x14, data, sizeof(data));  
        
    c0 = data[1]<<8 | data[0];
    c1 = data[3]<<8 | data[2];

    //see TSL2572 datasheet
    cpl = 51.87 * (float)gain_val / 60.0;
    if(GAIN_DIVIDE_6) cpl/=6.0;
    lux1 = ((float)c0 - (1.87 * (float)c1)) / cpl;
    lux2 = ((0.63 * (float)c0) - (float)c1) / cpl;
    cpl = max(lux1, lux2);
    return max(cpl, 0.0);
}
