
#ifndef LIGHT_SENSOR_TSL2572_H_
#define LIGHT_SENSOR_TSL2572_H_

#ifdef __cplusplus
extern "C" {
#endif

#define TSL25721            0x34
#define TSL2572_I2CADDR     0x39
#define I2C_DEV_TSL2572    "I2C_2"

#define GAIN_1X 0
#define GAIN_8X 1
#define GAIN_16X 2
#define GAIN_120X 3

#define GAIN_DIVIDE_6 true 

#define TSL2572_CMD_REGISTER        0x80

#define TSL2572_CMDS_ENABLE         0x00
#define TSL2572_CMDS_ALS_TIMING     0x01
#define TSL2572_CMDS_WAIT_TIME      0x03
#define TSL2572_CMDS_PERSISTANCE    0x0c
#define TSL2572_CMDS_CONFIG         0x0d
#define TSL2572_CMDS_CONTROL        0x0f
#define TSL2572_CMDS_WHOAMI         0x12
#define TSL2572_CMDS_STATUS         0x13




int iotex_TSL2572_init(uint8_t gain);
float iotex_Tsl2572ReadAmbientLight(void);


#ifdef __cplusplus
}
#endif
/**
 * @}
 */

#endif /* LIGHT_SENSOR_TSL2572_H_ */