/*
 * ________________________________________________________________________________________________________
 * Copyright (c) 2015-2015 InvenSense Inc. All rights reserved.
 *
 * This software, related documentation and any modifications thereto (collectively “Software”) is subject
 * to InvenSense and its licensors' intellectual property rights under U.S. and international copyright
 * and other intellectual property rights laws.
 *
 * InvenSense and its licensors retain all intellectual property and proprietary rights in and to the Software
 * and any use, reproduction, disclosure or distribution of the Software without an express license agreement
 * from InvenSense is strictly prohibited.
 *
 * EXCEPT AS OTHERWISE PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, THE SOFTWARE IS
 * PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * EXCEPT AS OTHERWISE PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, IN NO EVENT SHALL
 * INVENSENSE BE LIABLE FOR ANY DIRECT, SPECIAL, INDIRECT, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THE SOFTWARE.
 * ________________________________________________________________________________________________________
 */

#include "Icm426xxSelfTest.h"
#include "Icm426xxDefs.h"
#include "Icm426xxExtFunc.h"
#include "Icm426xxTransport.h"
#include "Icm426xxDriver_HL.h"

#ifndef INV_ABS
#define INV_ABS(x) (((x) < 0) ? -(x) : (x))
#endif

#define DEFAULT_ST_GYRO_FSR_DPS    250
#define DEFAULT_ST_GYRO_FSR_ENUM   ICM426XX_GYRO_CONFIG0_FS_SEL_250dps
#define DEFAULT_ST_ACCEL_FSR_G     2
#define DEFAULT_ST_ACCEL_FSR_ENUM  ICM426XX_ACCEL_CONFIG0_FS_SEL_2g

#define MIN_DEFAULT_ST_GYRO_DPS  60 /* expected values greater than 60dps */
#define MAX_DEFAULT_ST_GYRO_OFFSET_DPS 20 /* expected offset less than 20 dps */

#define MIN_DEFAULT_ST_ACCEL_MG 225 /* expected values in [225mgee;675mgee] */
#define MAX_DEFAULT_ST_ACCEL_MG 675

/** register configuration for self-test procedure */
#define DEFAULT_ST_GYRO_DEC2_M2_ORD      2
#define DEFAULT_ST_GYRO_UI_FILT_ORD_IND  ICM426XX_GYRO_CONFIG_GYRO_UI_FILT_ORD_3RD_ORDER
#define DEFAULT_ST_GYRO_UI_FILT_BW_IND   ICM426XX_GYRO_ACCEL_CONFIG0_GYRO_FILT_BW_10

#define DEFAULT_ST_ACCEL_UI_FILT_ORD_IND ICM426XX_ACCEL_CONFIG_ACCEL_UI_FILT_ORD_3RD_ORDER
#define DEFAULT_ST_ACCEL_UI_FILT_BW_IND  ICM426XX_GYRO_ACCEL_CONFIG0_ACCEL_FILT_BW_10

/** @brief Icm426xx HW Base sensor status based upon s->sensor_on_mask
 */
enum inv_icm426xx_sensor_on_mask {
	INV_ICM426XX_SENSOR_ON_MASK_ACCEL = (1L<<INV_ICM426XX_SENSOR_ACCEL),
	INV_ICM426XX_SENSOR_ON_MASK_GYRO = (1L<<INV_ICM426XX_SENSOR_GYRO),
	INV_ICM426XX_SENSOR_ON_MASK_EIS = (1L<<INV_ICM426XX_SENSOR_FSYNC_EVENT),
	INV_ICM426XX_SENSOR_ON_MASK_OIS = (1L<<INV_ICM426XX_SENSOR_OIS),
	INV_ICM426XX_SENSOR_ON_MASK_TEMP = (1L<<INV_ICM426XX_SENSOR_TEMPERATURE),
};

/** @brief Contains the current register values. Used to reapply values after the ST procedure
 */
struct recover_regs {
	/* bank 0 */
	uint8_t intf_config1;       /* REG_INTF_CONFIG1       */
	uint8_t pwr_mgmt_0;         /* REG_PWR_MGMT_0         */
	uint8_t accel_config0;      /* REG_ACCEL_CONFIG0      */
	uint8_t accel_config1;      /* REG_ACCEL_CONFIG1      */
	uint8_t gyro_config0;       /* REG_GYRO_CONFIG0       */
	uint8_t gyro_config1;       /* REG_GYRO_CONFIG1       */
	uint8_t accel_gyro_config0; /* REG_ACCEL_GYRO_CONFIG0 */
	uint8_t fifo_config1;       /* REG_FIFO_CONFIG1       */
	uint8_t self_test_config;   /* REG_SELF_TEST_CONFIG   */
};

static const uint16_t sSelfTestEquation[256] = {
	2620, 2646, 2672, 2699, 2726, 2753, 2781, 2808,
	2837, 2865, 2894, 2923, 2952, 2981, 3011, 3041,
	3072, 3102, 3133, 3165, 3196, 3228, 3261, 3293,
	3326, 3359, 3393, 3427, 3461, 3496, 3531, 3566,
	3602, 3638, 3674, 3711, 3748, 3786, 3823, 3862,
	3900, 3939, 3979, 4019, 4059, 4099, 4140, 4182,
	4224, 4266, 4308, 4352, 4395, 4439, 4483, 4528,
	4574, 4619, 4665, 4712, 4759, 4807, 4855, 4903,
	4953, 5002, 5052, 5103, 5154, 5205, 5257, 5310,
	5363, 5417, 5471, 5525, 5581, 5636, 5693, 5750,
	5807, 5865, 5924, 5983, 6043, 6104, 6165, 6226,
	6289, 6351, 6415, 6479, 6544, 6609, 6675, 6742,
	6810, 6878, 6946, 7016, 7086, 7157, 7229, 7301,
	7374, 7448, 7522, 7597, 7673, 7750, 7828, 7906,
	7985, 8065, 8145, 8227, 8309, 8392, 8476, 8561,
	8647, 8733, 8820, 8909, 8998, 9088, 9178, 9270,
	9363, 9457, 9551, 9647, 9743, 9841, 9939, 10038,
	10139, 10240, 10343, 10446, 10550, 10656, 10763, 10870,
	10979, 11089, 11200, 11312, 11425, 11539, 11654, 11771,
	11889, 12008, 12128, 12249, 12371, 12495, 12620, 12746,
	12874, 13002, 13132, 13264, 13396, 13530, 13666, 13802,
	13940, 14080, 14221, 14363, 14506, 14652, 14798, 14946,
	15096, 15247, 15399, 15553, 15709, 15866, 16024, 16184,
	16346, 16510, 16675, 16842, 17010, 17180, 17352, 17526,
	17701, 17878, 18057, 18237, 18420, 18604, 18790, 18978,
	19167, 19359, 19553, 19748, 19946, 20145, 20347, 20550,
	20756, 20963, 21173, 21385, 21598, 21814, 22033, 22253,
	22475, 22700, 22927, 23156, 23388, 23622, 23858, 24097,
	24338, 24581, 24827, 25075, 25326, 25579, 25835, 26093,
	26354, 26618, 26884, 27153, 27424, 27699, 27976, 28255,
	28538, 28823, 29112, 29403, 29697, 29994, 30294, 30597,
	30903, 31212, 31524, 31839, 32157, 32479, 32804
};

static int inv_icm426xx_reg_2_accel_fsr(ICM426XX_ACCEL_CONFIG0_FS_SEL_t reg)
{
	switch(reg) {
	case ICM426XX_ACCEL_CONFIG0_FS_SEL_2g:   return 2;
	case ICM426XX_ACCEL_CONFIG0_FS_SEL_4g:   return 4;
	case ICM426XX_ACCEL_CONFIG0_FS_SEL_8g:   return 8;
	case ICM426XX_ACCEL_CONFIG0_FS_SEL_16g:  return 16;
#if defined(ICM42686)
	case ICM426XX_ACCEL_CONFIG0_FS_SEL_32g:  return 32;
#endif
	default:          return -1;
	}
}
static int inv_icm426xx_reg_2_gyro_fsr(ICM426XX_GYRO_CONFIG0_FS_SEL_t reg)
{
	switch(reg) {
#if !defined(ICM42686)
	case ICM426XX_GYRO_CONFIG0_FS_SEL_16dps:   return 16;
#endif
	case ICM426XX_GYRO_CONFIG0_FS_SEL_31dps:   return 31;
	case ICM426XX_GYRO_CONFIG0_FS_SEL_62dps:   return 62;
	case ICM426XX_GYRO_CONFIG0_FS_SEL_125dps:  return 125;
	case ICM426XX_GYRO_CONFIG0_FS_SEL_250dps:  return 250;
	case ICM426XX_GYRO_CONFIG0_FS_SEL_500dps:  return 500;
	case ICM426XX_GYRO_CONFIG0_FS_SEL_1000dps: return 1000;
	case ICM426XX_GYRO_CONFIG0_FS_SEL_2000dps: return 2000;
#if defined(ICM42686)
	case ICM426XX_GYRO_CONFIG0_FS_SEL_4000dps: return 4000;
#endif
	default:             return -1;
	}
}

static int save_settings(struct inv_icm426xx * s, struct recover_regs * saved_regs)
{
	int result = 0;

	result |= inv_icm426xx_read_reg(s, MPUREG_INTF_CONFIG1, 1, &saved_regs->intf_config1);
	result |= inv_icm426xx_read_reg(s, MPUREG_PWR_MGMT_0, 1, &saved_regs->pwr_mgmt_0);
	result |= inv_icm426xx_read_reg(s, MPUREG_ACCEL_CONFIG0, 1, &saved_regs->accel_config0);
	result |= inv_icm426xx_read_reg(s, MPUREG_ACCEL_CONFIG1, 1, &saved_regs->accel_config1);
	result |= inv_icm426xx_read_reg(s, MPUREG_GYRO_CONFIG0, 1, &saved_regs->gyro_config0);
	result |= inv_icm426xx_read_reg(s, MPUREG_GYRO_CONFIG1, 1, &saved_regs->gyro_config1);
	result |= inv_icm426xx_read_reg(s, MPUREG_ACCEL_GYRO_CONFIG0, 1, &saved_regs->accel_gyro_config0);
	result |= inv_icm426xx_read_reg(s, MPUREG_FIFO_CONFIG1, 1, &saved_regs->fifo_config1);
	result |= inv_icm426xx_read_reg(s, MPUREG_SELF_TEST_CONFIG, 1, &saved_regs->self_test_config);

	return result;
}

static int recover_settings(struct inv_icm426xx * s, const struct recover_regs * saved_regs)
{
	int result = 0;

	/* Set en_g{x/y/z}_st_d2a to 0 disable self-test for each axis */
	result |= inv_icm426xx_write_reg(s, MPUREG_SELF_TEST_CONFIG, 1, &saved_regs->self_test_config);
	/*Restore gyro_dec2_m2_ord, gyro_ui_filt_ord_ind and gyro_ui_filt_bw_ind to previous values.*/
	result |= inv_icm426xx_write_reg(s, MPUREG_INTF_CONFIG1, 1, &saved_regs->intf_config1);
	result |= inv_icm426xx_write_reg(s, MPUREG_PWR_MGMT_0, 1, &saved_regs->pwr_mgmt_0);
	result |= inv_icm426xx_write_reg(s, MPUREG_ACCEL_CONFIG0, 1, &saved_regs->accel_config0);
	result |= inv_icm426xx_write_reg(s, MPUREG_ACCEL_CONFIG1, 1, &saved_regs->accel_config1);
	result |= inv_icm426xx_write_reg(s, MPUREG_GYRO_CONFIG0, 1, &saved_regs->gyro_config0);
	result |= inv_icm426xx_write_reg(s, MPUREG_GYRO_CONFIG1, 1, &saved_regs->gyro_config1);
	result |= inv_icm426xx_write_reg(s, MPUREG_FIFO_CONFIG1, 1, &saved_regs->fifo_config1);
	result |= inv_icm426xx_write_reg(s, MPUREG_ACCEL_GYRO_CONFIG0, 1, &saved_regs->accel_gyro_config0);
	/* wait 200ms for gyro output to settle */
	inv_icm426xx_sleep_us(200*1000);

	result |= inv_icm426xx_reset_fifo(s);

	return result;
}

static int do_average_on_sensor_output(struct inv_icm426xx * s, int sensor, int self_test_flag, int32_t sensor_result[3])
{
	int result = 0;
	int it = 0, sample_discarded = 0, timeout;
	uint8_t data, data_pwr_mgmt, reg, ST_sensor_data[6];
	int16_t ST_buffer[3] = {0};
	int32_t ST_sum[3] = {0};

	if(sensor == INV_ICM426XX_SENSOR_ON_MASK_GYRO) {
		reg = MPUREG_GYRO_DATA_X0_UI;

		/* Enable Gyro */
		result |= inv_icm426xx_read_reg(s, MPUREG_PWR_MGMT_0, 1, &data_pwr_mgmt);
		data_pwr_mgmt &= (uint8_t)~BIT_PWR_MGMT_0_GYRO_MODE_MASK;
		data_pwr_mgmt |= (uint8_t)ICM426XX_PWR_MGMT_0_GYRO_MODE_LN;
		result |= inv_icm426xx_write_reg(s, MPUREG_PWR_MGMT_0, 1, &data_pwr_mgmt);

		/* wait for 60ms to allow output to settle */
		inv_icm426xx_sleep_us(60*1000);
	}
	else if(sensor == INV_ICM426XX_SENSOR_ON_MASK_ACCEL) {
		reg = MPUREG_ACCEL_DATA_X0_UI;
		
		/* Enable Accel */
		result |= inv_icm426xx_read_reg(s, MPUREG_PWR_MGMT_0, 1, &data_pwr_mgmt);
		data_pwr_mgmt &= (uint8_t)~BIT_PWR_MGMT_0_ACCEL_MODE_MASK;
		data_pwr_mgmt |= (uint8_t)ICM426XX_PWR_MGMT_0_ACCEL_MODE_LN;
		result |= inv_icm426xx_write_reg(s, MPUREG_PWR_MGMT_0, 1, &data_pwr_mgmt);

		/* wait for 25ms to allow output to settle */
		inv_icm426xx_sleep_us(25*1000);
	}
	else
		return result;

	if(self_test_flag) {
		inv_icm426xx_read_reg(s, MPUREG_SELF_TEST_CONFIG, 1, &data);
		data |= self_test_flag; 
		inv_icm426xx_write_reg(s, MPUREG_SELF_TEST_CONFIG, 1, &data);

		if(sensor == INV_ICM426XX_SENSOR_ON_MASK_GYRO)
			/* wait 200ms for the oscillation to stabilize */
			inv_icm426xx_sleep_us(200*1000);
		else 
			/* wait for 25ms to allow output to settle */
			inv_icm426xx_sleep_us(25*1000);
	}

	timeout = 200;
	do {
		uint8_t int_status;
		inv_icm426xx_read_reg(s, MPUREG_INT_STATUS, 1, &int_status);
		
		if (int_status & BIT_INT_STATUS_DRDY) {
			inv_icm426xx_read_reg(s, reg, 6, ST_sensor_data);
			if (s->endianess_data == ICM426XX_INTF_CONFIG0_DATA_BIG_ENDIAN) {
				ST_buffer[0] = (ST_sensor_data[0] << 8) | ST_sensor_data[1];
				ST_buffer[1] = (ST_sensor_data[2] << 8) | ST_sensor_data[3];
				ST_buffer[2] = (ST_sensor_data[4] << 8) | ST_sensor_data[5];
			} else { // LITTLE ENDIAN
				ST_buffer[0] = (ST_sensor_data[1] << 8) | ST_sensor_data[0];
				ST_buffer[1] = (ST_sensor_data[3] << 8) | ST_sensor_data[2];
				ST_buffer[2] = (ST_sensor_data[5] << 8) | ST_sensor_data[4];
			}
			if ((ST_buffer[0] != -32768) && (ST_buffer[1] != -32768) && (ST_buffer[2] != -32768)) {
				ST_sum[0] += ST_buffer[0];
				ST_sum[1] += ST_buffer[1];
				ST_sum[2] += ST_buffer[2];
			} else {
				sample_discarded ++;
			}
			it ++;
		}
		inv_icm426xx_sleep_us(1000);
		timeout --;
	} while((it < 200) && (timeout > 0));

	/* Disable Accel and Gyro */
	result |= inv_icm426xx_read_reg(s, MPUREG_PWR_MGMT_0, 1, &data_pwr_mgmt);
	data_pwr_mgmt &= (uint8_t)~BIT_PWR_MGMT_0_GYRO_MODE_MASK;
	data_pwr_mgmt &= (uint8_t)~BIT_PWR_MGMT_0_ACCEL_MODE_MASK;
	data_pwr_mgmt |= (uint8_t)ICM426XX_PWR_MGMT_0_GYRO_MODE_OFF;
	data_pwr_mgmt |= (uint8_t)ICM426XX_PWR_MGMT_0_ACCEL_MODE_OFF;
	result |= inv_icm426xx_write_reg(s, MPUREG_PWR_MGMT_0, 1, &data_pwr_mgmt);

	it -= sample_discarded;
	sensor_result[0] = (ST_sum[0] / it);
	sensor_result[1] = (ST_sum[1] / it);
	sensor_result[2] = (ST_sum[2] / it);
	
	if(self_test_flag) {
		data &= ~self_test_flag;
		inv_icm426xx_write_reg(s, MPUREG_SELF_TEST_CONFIG, 1, &data);
	}

	return result;
}

/*
 * check_gyro_self_test
 * Param  result: 1 if success, 0 if failure
 * Returns 0 if success, error code if failure
 */
static int check_gyro_self_test(struct inv_icm426xx * s, struct recover_regs * saved_reg, int * result)
{
	int rc = 0, i = 0, otp_value_zero = 0;
	uint8_t data, bank, regs[3];
	int32_t ST_OFF_gyro[3], ST_ON_gyro[3];
	uint32_t ST_gyro_res[3], ST_gyro_otp[3];
	uint32_t gyro_sensitivity_1dps;
	ICM426XX_GYRO_CONFIG0_FS_SEL_t fsr = ICM426XX_GYRO_CONFIG0_FS_SEL_250dps;
	
	*result = 1;

	/** Check Gyro self-test response */

	/* set configuration values to set ODR to 1kHz and bandwith to 100Hz */
	data = saved_reg->gyro_config1; 
	data &= ~BIT_GYRO_CONFIG1_GYRO_UI_FILT_ORD_MASK;
	data &= ~BIT_GYRO_CONFIG1_GYRO_DEC2_M2_ORD_MASK;
	data |= DEFAULT_ST_GYRO_UI_FILT_ORD_IND;
	data |= DEFAULT_ST_GYRO_DEC2_M2_ORD;
	rc |= inv_icm426xx_write_reg(s, MPUREG_GYRO_CONFIG1, 1, &data);
	
	data = saved_reg->accel_gyro_config0; 
	data &= ~BIT_GYRO_ACCEL_CONFIG0_GYRO_FILT_MASK; 
	data |= DEFAULT_ST_GYRO_UI_FILT_BW_IND;
	rc |= inv_icm426xx_write_reg(s, MPUREG_ACCEL_GYRO_CONFIG0, 1, &data);
	
	data = saved_reg->gyro_config0;
	data &= ~BIT_GYRO_CONFIG0_FS_SEL_MASK;
	data &= ~BIT_GYRO_CONFIG0_ODR_MASK;
	data |= fsr;
	data |= ICM426XX_GYRO_CONFIG0_ODR_1_KHZ;
	rc |= inv_icm426xx_write_reg(s, MPUREG_GYRO_CONFIG0, 1, &data);
	
	/* read average gyro digital output for each axis and store them as ST_OFF_{x,y,z} in lsb */
	rc |= do_average_on_sensor_output(s, INV_ICM426XX_SENSOR_ON_MASK_GYRO, 0, ST_OFF_gyro);

	/* enable self-test for each axis */
	/* then read average gyro digital output for each axis and store them as ST_ON_{x,y,z} in lsb */
	rc |= do_average_on_sensor_output(s, INV_ICM426XX_SENSOR_ON_MASK_GYRO, (BIT_GYRO_X_ST_EN + BIT_GYRO_Y_ST_EN + BIT_GYRO_Z_ST_EN), ST_ON_gyro);

	/* calculate the self-test response as ABS(ST_ON_{x,y,z} - ST_OFF_{x,y,z}) for each axis */
	for(i = 0; i < 3; i++) {
		ST_gyro_res[i] = INV_ABS(ST_ON_gyro[i] - ST_OFF_gyro[i]);
	}

	/* test if ST results match the expected results */
	gyro_sensitivity_1dps = 32768 / inv_icm426xx_reg_2_gyro_fsr(fsr);

	/** Trim Gyro self-test response */

	/* calculate ST results OTP based on the equation */
	bank = 1;
	rc |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);
	rc |= inv_icm426xx_read_reg(s, MPUREG_XG_ST_DATA_B1, 3, regs);
	for (i = 0; i < 3; i++) {
		if (regs[i] != 0) {
			ST_gyro_otp[i] = sSelfTestEquation[regs[i] - 1];
		} else {
			ST_gyro_otp[i] = 0;
			otp_value_zero = 1;
		}
	}
	bank = 0;
	rc |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	/** Check Gyro self-test results */

	/* verify 'PASS' criteria:
	 * - (a) GST/GST_OTP > 0.5 or (b) GST >= 60dps,
	 * - GOFFSET <= 20dps
	 */
	for (i = 0; i < 3; i++) {
		if (!otp_value_zero) {
			float ratio = ((float)ST_gyro_res[i]) / ((float)ST_gyro_otp[i]);
			if (ratio <= 0.5f)
				*result = 0; /* fail */
		} else if(ST_gyro_res[i] < (MIN_DEFAULT_ST_GYRO_DPS * gyro_sensitivity_1dps)) {
			*result = 0; /* fail */
		}
	}

	/* stored the computed bias (checking GST and GOFFSET values) */
	for (i = 0; i < 3; i++) {
		if((INV_ABS(ST_OFF_gyro[i]) > (int32_t)(MAX_DEFAULT_ST_GYRO_OFFSET_DPS * gyro_sensitivity_1dps)))
			*result = 0; /* fail */
		s->gyro_st_bias[i] = ST_OFF_gyro[i];
	}

	return rc;
}

/*
 * check_accel_self_test
 * Param  result: 1 if success, 0 if failure
 * Returns 0 if success, error code if failure
 */
static int check_accel_self_test(struct inv_icm426xx * s, struct recover_regs * saved_reg, int * result)
{
	int rc = 0, i, otp_value_zero = 0, axis, axis_sign;
	uint8_t data, bank, regs[3];
	int32_t ST_OFF_accel[3], ST_ON_accel[3];
	uint32_t ST_accel_res[3], ST_accel_otp[3];
	uint32_t acc_sensitivity_1g, gravity; 
	ICM426XX_ACCEL_CONFIG0_FS_SEL_t fsr = ICM426XX_ACCEL_CONFIG0_FS_SEL_2g;

	*result = 1;
	
	/** Check Accel self-test */
	
	/* set configuration values to set ODR to 1kHz and bandwith to 100Hz */
	data = saved_reg->accel_gyro_config0; 
	data &= ~BIT_GYRO_ACCEL_CONFIG0_ACCEL_FILT_MASK;
	data |= DEFAULT_ST_ACCEL_UI_FILT_BW_IND;
	rc |= inv_icm426xx_write_reg(s, MPUREG_ACCEL_GYRO_CONFIG0, 1, &data);
	
	data = saved_reg->accel_config1;
	data &= ~BIT_ACCEL_CONFIG1_ACCEL_UI_FILT_ORD_MASK;
	data |= DEFAULT_ST_ACCEL_UI_FILT_ORD_IND;
	rc |= inv_icm426xx_write_reg(s, MPUREG_ACCEL_CONFIG1, 1, &data);
	
	data = saved_reg->accel_config0;
	data &= ~BIT_ACCEL_CONFIG0_FS_SEL_MASK;
	data &= ~BIT_ACCEL_CONFIG0_ODR_MASK;
	data |= fsr;
	data |= ICM426XX_ACCEL_CONFIG0_ODR_1_KHZ;
	rc |= inv_icm426xx_write_reg(s, MPUREG_ACCEL_CONFIG0, 1, &data);

	/* read average accel digital output for each axis and store them as ST_OFF_{x,y,z} in lsb x 1000 */
	rc |= do_average_on_sensor_output(s, INV_ICM426XX_SENSOR_ON_MASK_ACCEL, 0, ST_OFF_accel);

	/* enable self-test for each axis */
	/* then read average gyro digital output for each axis and store them as ST_ON_{x,y,z} in lsb x 1000 */
	rc |= do_average_on_sensor_output(s, INV_ICM426XX_SENSOR_ON_MASK_ACCEL, (BIT_ACCEL_X_ST_EN + BIT_ACCEL_Y_ST_EN + BIT_ACCEL_Z_ST_EN + BIT_ST_REGULATOR_EN), ST_ON_accel);

	/* calculate the self-test response as ABS(ST_ON_{x,y,z} - ST_OFF_{x,y,z}) for each axis */
	/* outputs from this routine are in units of lsb and hence are dependent on the full-scale used on the DUT */
	for(i = 0; i < 3; i++) {
		ST_accel_res[i] = INV_ABS(ST_ON_accel[i] - ST_OFF_accel[i]);
	}

	/* test if ST results match the expected results */
	acc_sensitivity_1g = 32768 / inv_icm426xx_reg_2_accel_fsr(fsr);

	/** Trim Accel self-test */

	/* calculate ST results OTP based on the equation */
	bank = 2;
	rc |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);
	rc |= inv_icm426xx_read_reg(s, MPUREG_XA_ST_DATA_B2, 3, regs);
	for (i = 0; i < 3; i++) {
		if (regs[i] != 0) {
			ST_accel_otp[i] = sSelfTestEquation[regs[i] - 1];
		} else {
			ST_accel_otp[i] = 0;
			otp_value_zero = 1;
		}
	}
	bank = 0;
	rc |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	/** Check Accel self-test result */

	/* verify 'PASS' criteria:
	 * - (a) 0.5 < AST/AST_OTP < 1.5 or,
	 * - (b) 225mgee <= AST <= 675mgee
	 */
	for (i = 0; i < 3; i++) {
		if (!otp_value_zero) {
			float ratio = ((float)ST_accel_res[i]) / ((float)ST_accel_otp[i]);
			if ((ratio >= 1.5f) || (ratio <= 0.5f))
				*result = 0; /* fail */
		} else if ((ST_accel_res[i] < ((MIN_DEFAULT_ST_ACCEL_MG * acc_sensitivity_1g) / 1000))
				|| (ST_accel_res[i] > ((MAX_DEFAULT_ST_ACCEL_MG * acc_sensitivity_1g) / 1000))) {
			*result = 0; /* fail */
		}
	}

	/* stored the computed offset */
	for(i = 0; i < 3; i++) {
		s->accel_st_bias[i] = ST_OFF_accel[i];
	}

	/* assume the largest data axis shows +1 or -1 gee for gravity */
	axis = 0;
	axis_sign = 1;
	if (INV_ABS(s->accel_st_bias[1]) > INV_ABS(s->accel_st_bias[0]))
		axis = 1;
	if (INV_ABS(s->accel_st_bias[2]) > INV_ABS(s->accel_st_bias[axis]))
		axis = 2;
	if (s->accel_st_bias[axis] < 0)
		axis_sign = -1;

	gravity = acc_sensitivity_1g * axis_sign;
	s->accel_st_bias[axis] -= gravity;

	return rc;
}

static int set_offsetreg(struct inv_icm426xx * s, uint8_t sensor)
{
	uint8_t data[5];
	int16_t cur_bias;
	int status = 0;

	// Set memory bank 4
	status |= inv_icm426xx_set_reg_bank(s, 4);

	/* Set offset registers sensor */ 
	if (sensor == INV_ICM426XX_SENSOR_ON_MASK_ACCEL) {
		/* 
		 * Invert sign for OFFSET and
		 * accel_st_bias is 2g coded 16 
		 * OFFUSER is 1g coded 12 (or 2g coded 12 for High FSR parts)
		 */
		status |= inv_icm426xx_read_reg(s, MPUREG_OFFSET_USER_4_B4, 1, &data[0]); // Fetch gyro_z_offuser[8-11]
		data[0] &= BIT_GYRO_Z_OFFUSER_MASK_HI;
		
		cur_bias = (int16_t)(-s->accel_st_bias[0] >> 3);
		cur_bias /= ACCEL_OFFUSER_MAX_MG/1000;
		data[0] |= (((cur_bias & 0x0F00) >> 8) << BIT_ACCEL_X_OFFUSER_POS_HI);
		data[1] = ((cur_bias & 0x00FF) << BIT_ACCEL_X_OFFUSER_POS_LO);
		cur_bias = (int16_t)(-s->accel_st_bias[1] >> 3);
		cur_bias /= ACCEL_OFFUSER_MAX_MG/1000;
		data[2] = ((cur_bias & 0x00FF) << BIT_ACCEL_Y_OFFUSER_POS_LO);
		data[3] = (((cur_bias & 0x0F00) >> 8) << BIT_ACCEL_Y_OFFUSER_POS_HI);
		cur_bias = (int16_t)(-s->accel_st_bias[2] >> 3);
		cur_bias /= ACCEL_OFFUSER_MAX_MG/1000;
		data[3] |= (((cur_bias & 0x0F00) >> 8) << BIT_ACCEL_Z_OFFUSER_POS_HI);
		data[4] = ((cur_bias & 0x00FF) << BIT_ACCEL_Z_OFFUSER_POS_LO);
		
		status |= inv_icm426xx_write_reg(s, MPUREG_OFFSET_USER_4_B4, 5, &data[0]);
		
	} else if (sensor == INV_ICM426XX_SENSOR_ON_MASK_GYRO) {
		/* 
		 * Invert sign for OFFSET and
		 * gyro_st_bias is 250dps coded 16 
		 * OFFUSER is 64dps coded 12 (or 128dps coded 12 for High FSR parts)
		 */
		status |= inv_icm426xx_read_reg(s, MPUREG_OFFSET_USER_4_B4, 1, &data[4]); // Fetch accel_x_offuser[8-11]
		data[4] &= BIT_ACCEL_X_OFFUSER_MASK_HI;
		
		cur_bias = (int16_t)(-(s->gyro_st_bias[0]*250/GYRO_OFFUSER_MAX_DPS) >> 4);
		data[0] = ((cur_bias & 0x00FF) << BIT_GYRO_X_OFFUSER_POS_LO);
		data[1] = (((cur_bias & 0x0F00) >> 8) << BIT_GYRO_X_OFFUSER_POS_HI);
		cur_bias = (int16_t)(-(s->gyro_st_bias[1]*250/GYRO_OFFUSER_MAX_DPS) >> 4);
		data[1] |= (((cur_bias & 0x0F00) >> 8) << BIT_GYRO_Y_OFFUSER_POS_HI);
		data[2] = ((cur_bias & 0x00FF) << BIT_GYRO_Y_OFFUSER_POS_LO);
		cur_bias = (int16_t)(-(s->gyro_st_bias[2]*250/GYRO_OFFUSER_MAX_DPS) >> 4);
		data[3] = ((cur_bias & 0x00FF) << BIT_GYRO_Z_OFFUSER_POS_LO);
		data[4] |= (((cur_bias & 0x0F00) >> 8) << BIT_GYRO_Z_OFFUSER_POS_HI);
		
		status |= inv_icm426xx_write_reg(s, MPUREG_OFFSET_USER_0_B4, 5, &data[0]);
		
	}

	// Set memory bank 0
	status |= inv_icm426xx_set_reg_bank(s, 0);

	return status;
}

int inv_icm426xx_run_selftest(struct inv_icm426xx * s, int * result)
{
	int status = 0, gyro_result = 0, accel_result = 0;
	struct recover_regs saved_regs;

	*result = 0;

	/* Run self-test only once */
	if (s->st_result == 0) {

		/* Save current settings to restore them at the end of the routine */
		status |= save_settings(s, &saved_regs);

		status |= check_gyro_self_test(s, &saved_regs, &gyro_result);
		if ((status == 0) && (gyro_result == 1))
			status |= set_offsetreg(s, INV_ICM426XX_SENSOR_ON_MASK_GYRO);
		
		status |= check_accel_self_test(s, &saved_regs, &accel_result);
		if ((status == 0) && (accel_result == 1))
			status |= set_offsetreg(s, INV_ICM426XX_SENSOR_ON_MASK_ACCEL);

		/* Restore settings previously saved */
		status |= recover_settings(s, &saved_regs);
		
		/* Store acc and gyr results */
		s->st_result = (accel_result << 1) | gyro_result;
	}

	*result = s->st_result;
	return status;
}

void inv_icm426xx_get_st_bias(struct inv_icm426xx * s, int st_bias[6])
{
	int i;
	
	/* if ST didn't run, return null biases */
	if (s->st_result == 0) {
		for (i = 0; i < 6; i++)
			st_bias[i] = 0;
		return;
	}

	/* Gyro bias LN: first 3 elements */
	for (i = 0; i < 3; i++) /* convert bias to 1 dps Q16 */
		st_bias[i] = s->gyro_st_bias[i] * 2 * DEFAULT_ST_GYRO_FSR_DPS;

	/* Accel bias LN: last 3 elements */
	for (i = 0; i < 3; i++) /* convert bias to 1 gee Q16 */
		st_bias[i+3] = s->accel_st_bias[i] * 2 * DEFAULT_ST_ACCEL_FSR_G;
}

void inv_icm426xx_set_st_bias(struct inv_icm426xx * s, const int st_bias[6])
{
	int i;
	
	/* Gyro */
	for (i = 0; i < 3; i++)
		s->gyro_st_bias[i] = st_bias[i] / (2 * DEFAULT_ST_GYRO_FSR_DPS);

	set_offsetreg(s, INV_ICM426XX_SENSOR_ON_MASK_GYRO);
	
	/* Accel */
	for (i = 0; i < 3; i++)
		s->accel_st_bias[i] = st_bias[i+3] / (2 * DEFAULT_ST_ACCEL_FSR_G);

	set_offsetreg(s, INV_ICM426XX_SENSOR_ON_MASK_ACCEL);
}
