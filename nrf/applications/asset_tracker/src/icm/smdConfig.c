
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


// ******************************  wom / smd **************************************

/*
 * Set power mode flag
 * Set this flag to run example in low-noise mode.
 * Reset this flag to run example in low-power mode.
 * Note : low-noise mode is not available with sensor data frequencies less than 12.5Hz.
 */
#define IS_LOW_NOISE_MODE 0



static int ConfigureInvDevice_smd(uint8_t is_low_noise_mode, ICM426XX_ACCEL_CONFIG0_FS_SEL_t acc_fsr_g, ICM426XX_ACCEL_CONFIG0_ODR_t acc_freq)
{
	int rc = 0;
	inv_icm426xx_interrupt_parameter_t config_int = {(inv_icm426xx_interrupt_value)0};
	
	rc |= inv_icm426xx_set_accel_fsr(getICMDriver(), acc_fsr_g);
	
	rc |= inv_icm426xx_set_accel_frequency(getICMDriver(), acc_freq);
	
	if (is_low_noise_mode)	{
		rc |= inv_icm426xx_enable_accel_low_noise_mode(getICMDriver());
	}else {
		/* Set 1x averaging, in order to minimize power consumption (16x by default) */
		rc |= inv_icm426xx_set_accel_lp_avg(getICMDriver(), ICM426XX_GYRO_ACCEL_CONFIG0_ACCEL_FILT_AVG_1);
		rc |= inv_icm426xx_enable_accel_low_power_mode(getICMDriver());
	}

	/* Configure WOM to compare current sample with the previous sample and to produce signal when all axis exceed 52 mg */
	rc |= inv_icm426xx_configure_smd_wom(getICMDriver(), 
		ICM426XX_DEFAULT_WOM_THS_MG, 
		ICM426XX_DEFAULT_WOM_THS_MG, 
		ICM426XX_DEFAULT_WOM_THS_MG, 
		ICM426XX_SMD_CONFIG_WOM_INT_MODE_ANDED,
		ICM426XX_SMD_CONFIG_WOM_MODE_CMP_PREV);

	rc |= inv_icm426xx_enable_smd(getICMDriver());

	/* Disable fifo threshold, data ready and WOM interrupts INT1 */
	inv_icm426xx_get_config_int1(getICMDriver(), &config_int);
	config_int.INV_ICM426XX_FIFO_THS = INV_ICM426XX_DISABLE;
	config_int.INV_ICM426XX_UI_DRDY = INV_ICM426XX_DISABLE;
	config_int.INV_ICM426XX_WOM_X = INV_ICM426XX_DISABLE;
	config_int.INV_ICM426XX_WOM_Y = INV_ICM426XX_DISABLE;
	config_int.INV_ICM426XX_WOM_Z = INV_ICM426XX_DISABLE;
	inv_icm426xx_set_config_int1(getICMDriver(), &config_int);

	/* Disable fifo threshold, data ready and WOM interrupts IBI */
	inv_icm426xx_get_config_ibi(getICMDriver(), &config_int);
	config_int.INV_ICM426XX_FIFO_THS = INV_ICM426XX_DISABLE;
	config_int.INV_ICM426XX_UI_DRDY = INV_ICM426XX_DISABLE;
	config_int.INV_ICM426XX_WOM_X = INV_ICM426XX_DISABLE;
	config_int.INV_ICM426XX_WOM_Y = INV_ICM426XX_DISABLE;
	config_int.INV_ICM426XX_WOM_Z = INV_ICM426XX_DISABLE;
	inv_icm426xx_set_config_ibi(getICMDriver(), &config_int);

	return rc;
}


int smdConf(void)
{
    return ConfigureInvDevice_smd((uint8_t)IS_LOW_NOISE_MODE, ICM426XX_ACCEL_CONFIG0_FS_SEL_4g, ICM426XX_ACCEL_CONFIG0_ODR_50_HZ);
}

int smdDetect(void)
{
	int status;
	uint8_t int_status;
	
    status |= inv_icm426xx_read_reg(getICMDriver(), MPUREG_INT_STATUS2, 1, &int_status);
    if (status) {
        printk("error smd status read : 0x%x\n",int_status);        
        return  0;
    }	
	if(int_status & (BIT_INT_STATUS2_SMD_INT)) {
		printk("smd event detected\n");
	}
	return 1;
}

