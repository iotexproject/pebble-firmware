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

/* Initial WOM threshold to be applied to ICM in mg */
#define WOM_THRESHOLD_INITIAL_MG 200

/* WOM threshold to be applied to ICM, ranges from 1 to 255, in 4mg unit */
static uint8_t wom_threshold = WOM_THRESHOLD_INITIAL_MG/4;

static int ConfigureInvDevice_wom(uint8_t is_low_noise_mode, 
                       ICM426XX_ACCEL_CONFIG0_FS_SEL_t acc_fsr_g,
                       ICM426XX_ACCEL_CONFIG0_ODR_t acc_freq)
{
	int rc = 0;
	
	rc |= inv_icm426xx_set_accel_fsr(getICMDriver(), acc_fsr_g);
	
	rc |= inv_icm426xx_set_accel_frequency(getICMDriver(), acc_freq);
	
	if(is_low_noise_mode) {
		rc |= inv_icm426xx_enable_accel_low_noise_mode(getICMDriver());
	}
	else {
		/* Set 1x averaging, in order to minimize power consumption (16x by default) */
		rc |= inv_icm426xx_set_accel_lp_avg(getICMDriver(), ICM426XX_GYRO_ACCEL_CONFIG0_ACCEL_FILT_AVG_1);
		rc |= inv_icm426xx_enable_accel_low_power_mode(getICMDriver());
	}

	/* Configure WOM to produce signal when at least one axis exceed 200 mg */
	rc |= inv_icm426xx_configure_smd_wom(getICMDriver(), wom_threshold, wom_threshold, wom_threshold, ICM426XX_SMD_CONFIG_WOM_INT_MODE_ORED, ICM426XX_SMD_CONFIG_WOM_MODE_CMP_PREV);
	rc |= inv_icm426xx_enable_wom(getICMDriver());

	if (rc)
		printk("Error while configuring WOM threshold to initial value %c", wom_threshold);
	
	printk("Waiting for detection...");
	return rc;
}


int womConf(void)
{
	if(!IS_LOW_NOISE_MODE) {
		/*In case LPM is used, accel frequency is 12.5Hz */		
		return ConfigureInvDevice_wom((uint8_t )IS_LOW_NOISE_MODE, \
		       ICM426XX_ACCEL_CONFIG0_FS_SEL_4g, ICM426XX_ACCEL_CONFIG0_ODR_12_5_HZ);
	}
	else {
		/*In case LNM is used, accel frequency is 50Hz */
		return ConfigureInvDevice_wom((uint8_t )IS_LOW_NOISE_MODE, \
		       ICM426XX_ACCEL_CONFIG0_FS_SEL_4g, ICM426XX_ACCEL_CONFIG0_ODR_50_HZ);		
	}		
}

int womDetect(void)
{
	int status;
	uint8_t int_status;
	
    status |= inv_icm426xx_read_reg(getICMDriver(), MPUREG_INT_STATUS2, 1, &int_status);
    if (status) {
        printk("error wom status read : 0x%x\n",int_status);        
        return  0;
    }	
	if(int_status & (BIT_INT_STATUS2_WOM_X_INT|BIT_INT_STATUS2_WOM_Y_INT|BIT_INT_STATUS2_WOM_Z_INT)) {
		printk("wom event detected\n");
	}
	return 1;
}
