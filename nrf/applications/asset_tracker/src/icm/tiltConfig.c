#include <zephyr.h>
#include <stdio.h>
#include <stdlib.h>
#include <drivers/i2c.h>
#include "Icm426xxDefs.h"
#include "Icm426xxTransport.h"
#include "Icm426xxDriver_HL.h"
#include "Icm426xxDriver_HL_apex.h"
#include "icm42605_helper.h"
#include "modem/modem_helper.h"
#include "nvs/local_storage.h"

//  ************************ Tilt *******************************
/*
 * Tilt frequency 
 * Use type ICM426XX_APEX_CONFIG0_DMP_ODR_t to define tilt frequency
 * These types are defined in Icm426xxDefs.h.
 *
 * \note The frequency modes to run the Tilt are :
 * ICM426XX_APEX_CONFIG0_DMP_ODR_25Hz  (Low Power mode), 
 * ICM426XX_APEX_CONFIG0_DMP_ODR_50Hz  (High Performance mode)
 */
#define ICM_TILT_FREQUENCY_MODE ICM426XX_APEX_CONFIG0_DMP_ODR_25Hz

/*
 * Tilt wait time 
 * Use type ICM426XX_APEX_CONFIG4_TILT_WAIT_TIME_t to define the number of accelerometer samples 
 * to wait before triggering tilt event.
 * These types are defined in Icm426xxDefs.h.
 */
#define ICM_TILT_WAIT_TIME ICM426XX_APEX_CONFIG4_TILT_WAIT_TIME_4S

/*
 * Tilt power save mode
 * Use type ICM426XX_APEX_CONFIG0_DMP_POWER_SAVE_t to define tilt power save mode
 * These types are defined in Icm426xxDefs.h.
 */
#define ICM_TILT_POWER_SAVE_MODE  ICM426XX_APEX_CONFIG0_DMP_POWER_SAVE_EN

static int ConfigureInvDevice_tilt(ICM426XX_APEX_CONFIG0_DMP_ODR_t tilt_freq,
					   ICM426XX_APEX_CONFIG0_DMP_POWER_SAVE_t power_mode,
					   ICM426XX_APEX_CONFIG4_TILT_WAIT_TIME_t tilt_wait_time)
{
	int rc = 0;
	ICM426XX_ACCEL_CONFIG0_ODR_t acc_freq;
	inv_icm426xx_apex_parameters_t apex_inputs;
	inv_icm426xx_interrupt_parameter_t config_int = {(inv_icm426xx_interrupt_value)0};
	
	 /*	Accel frequency should be at least or higher than the Tilt frequency to properly running the APEX feature */
	switch(tilt_freq) {
		case ICM426XX_APEX_CONFIG0_DMP_ODR_25Hz:  acc_freq = ICM426XX_ACCEL_CONFIG0_ODR_25_HZ;  break;
		case ICM426XX_APEX_CONFIG0_DMP_ODR_50Hz:  acc_freq = ICM426XX_ACCEL_CONFIG0_ODR_50_HZ;  break;
		default: return INV_ERROR_BAD_ARG;
	}

	/* Enable accelerometer to feed the APEX Pedometer algorithm */
	rc |= inv_icm426xx_set_accel_frequency(getICMDriver(), acc_freq);

	/* Set 1x averaging, in order to minimize power consumption (16x by default) */
	rc |= inv_icm426xx_set_accel_lp_avg(getICMDriver(), ICM426XX_GYRO_ACCEL_CONFIG0_ACCEL_FILT_AVG_1);

	rc |= inv_icm426xx_enable_accel_low_power_mode(getICMDriver());

	/* Get the default parameters for the APEX features */
	rc |= inv_icm426xx_init_apex_parameters_struct(getICMDriver(), &apex_inputs);
	
	/* Configure the programmable parameter for number of accelerometer samples to wait before triggering tilt event */
	apex_inputs.tilt_wait_time = tilt_wait_time;
	
	/* Configure the programmable parameter for Low Power mode (WoM+Pedometer) or Normal mode */
	if (power_mode == ICM426XX_APEX_CONFIG0_DMP_POWER_SAVE_EN) {
		/* Configure WOM to compare current sample with the previous sample and to produce signal when all axis exceed 52 mg */
		rc |= inv_icm426xx_configure_smd_wom(getICMDriver(), 
			ICM426XX_DEFAULT_WOM_THS_MG, 
			ICM426XX_DEFAULT_WOM_THS_MG, 
			ICM426XX_DEFAULT_WOM_THS_MG, 
			ICM426XX_SMD_CONFIG_WOM_INT_MODE_ANDED,
			ICM426XX_SMD_CONFIG_WOM_MODE_CMP_PREV);

		/* Enable WOM to wake-up the DMP once it goes in power save mode */
		rc |= inv_icm426xx_enable_wom(getICMDriver()); /* Enable WOM and disable fifo threshold */

		apex_inputs.power_save = ICM426XX_APEX_CONFIG0_DMP_POWER_SAVE_EN;
	} else
		apex_inputs.power_save = ICM426XX_APEX_CONFIG0_DMP_POWER_SAVE_DIS;

	/* Initializes APEX features */
	rc |= inv_icm426xx_configure_apex_parameters(getICMDriver(), &apex_inputs);
	rc |= inv_icm426xx_set_apex_frequency(getICMDriver(), tilt_freq);

	/* Enable the tilt */
	rc |= inv_icm426xx_enable_apex_tilt(getICMDriver());

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
printk("tilt config rc : 0x%x\n",rc);	
	return rc;
}
int tiltConf(void)
{
    return ConfigureInvDevice_tilt(ICM_TILT_FREQUENCY_MODE,
							ICM_TILT_POWER_SAVE_MODE,
							ICM_TILT_WAIT_TIME);
}

int tiltDetect(void)
{
	int status;
	uint8_t int_status;
	
    status |= inv_icm426xx_read_reg(getICMDriver(), MPUREG_INT_STATUS3, 1, &int_status);
    if (status) {
        printk("error tilt status read : 0x%x\n",int_status);        
        return  0;
    }	
	if(int_status & BIT_INT_STATUS3_TILT_DET){
		printk("tilt event detected\n");
	}
	return 1;
}

