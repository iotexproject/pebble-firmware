/*
 * ________________________________________________________________________________________________________
 * Copyright (c) 2017 InvenSense Inc. All rights reserved.
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

/** @defgroup DriverIcm426xxUnit Icm426xx driver unit functions
 *  @brief Unit functions to access Icm426xx device registers
 *  @ingroup  DriverIcm426xx
 *  @{
 */

#include "Icm426xxDefsInternal.h"
#include "Icm426xxExtFunc.h"
#include "Icm426xxTransport.h"


/** @brief Set CHIP_ID register
 *
 *  @param[in] new_value 
 *  @return 0 in case of success, negative value on error. See enum inv_error
 */
int inv_icm426xx_wr_chip_id(struct inv_icm426xx * s, uint8_t new_value)
{
	return inv_icm426xx_write_reg(s, MPUREG_CHIP_ID, 1, &new_value);
}

/** @brief Set REV_ID register
 *
 *  @param[in] new_value 
 *  @return 0 in case of success, negative value on error. See enum inv_error
 */
int inv_icm426xx_wr_rev_id(struct inv_icm426xx * s, uint8_t new_value)
{
	return inv_icm426xx_write_reg(s, MPUREG_REV_ID, 1, &new_value);
}

/** @brief Get MPUREG_APEX_CONFIG0 register LOWG_EN bit
 *
 * @param[in] value See enum ICM426XX_APEX_CONFIG0_LOWG_EN_t
 * @return 0 in case of success, negative value on error. See enum inv_error
 */
int inv_icm426xx_rd_apex_config0_lowg_en(struct inv_icm426xx * s, ICM426XX_APEX_CONFIG0_LOWG_EN_t * value)
{
	int status;

	status = inv_icm426xx_read_reg(s, MPUREG_APEX_CONFIG0, 1, (uint8_t *)value);
	if(status)
		return status;

	*value &= BIT_APEX_CONFIG0_LOWG_EN_MASK;

	return status;
}

/** @brief Set MPUREG_APEX_CONFIG0 register LOWG_EN bit
 *
 * @param[in] new_value See enum ICM426XX_APEX_CONFIG0_LOWG_EN_t
 * @return 0 in case of success, negative value on error. See enum inv_error
 */
int inv_icm426xx_wr_apex_config0_lowg_en(struct inv_icm426xx * s, ICM426XX_APEX_CONFIG0_LOWG_EN_t new_value)
{
	uint8_t value;
	int status;

	status = inv_icm426xx_read_reg(s, MPUREG_APEX_CONFIG0, 1, &value);
	if(status)
		return status;

	value &= ~BIT_APEX_CONFIG0_LOWG_EN_MASK;
	value |= new_value;

	status = inv_icm426xx_write_reg(s, MPUREG_APEX_CONFIG0, 1, &value);
	return status;
}

/** @brief Get MPUREG_APEX_CONFIG0 register HIGHG_EN bit
 *
 * @param[in] value See enum ICM426XX_APEX_CONFIG0_HIGHG_EN_t
 * @return 0 in case of success, negative value on error. See enum inv_error
 */
int inv_icm426xx_rd_apex_config0_highg_en(struct inv_icm426xx * s, ICM426XX_APEX_CONFIG0_HIGHG_EN_t * value)
{
	int status;

	status = inv_icm426xx_read_reg(s, MPUREG_APEX_CONFIG0, 1, (uint8_t *)value);
	if(status)
		return status;

	*value &= BIT_APEX_CONFIG0_HIGHG_EN_MASK;

	return status;
}

/** @brief Set MPUREG_APEX_CONFIG0 register HIGHG_EN bit
 *
 * @param[in] new_value See enum ICM426XX_APEX_CONFIG0_HIGHG_EN_t
 * @return 0 in case of success, negative value on error. See enum inv_error
 */
int inv_icm426xx_wr_apex_config0_highg_en(struct inv_icm426xx * s, ICM426XX_APEX_CONFIG0_HIGHG_EN_t new_value)
{
	uint8_t value;
	int status;

	status = inv_icm426xx_read_reg(s, MPUREG_APEX_CONFIG0, 1, &value);
	if(status)
		return status;

	value &= ~BIT_APEX_CONFIG0_HIGHG_EN_MASK;
	value |= new_value;

	status = inv_icm426xx_write_reg(s, MPUREG_APEX_CONFIG0, 1, &value);
	return status;
}

/** @brief Set PWR_MGMT_0 register S4S_EN bit
 *
 *  <pre>
 *  0 – disable s4s
 *  1 – enable s4s
 *  </pre>
 * @param[in] new_value See enum ICM426XX_PWR_MGMT_0_S4S_t
 * @return 0 in case of success, negative value on error. See enum inv_error
 */
int inv_icm426xx_wr_pwr_mgmt0_s4s_en(struct inv_icm426xx * s, ICM426XX_PWR_MGMT_0_S4S_t new_value)
{
	uint8_t value;
	int status;

	status = inv_icm426xx_read_reg(s, MPUREG_PWR_MGMT_0, 1, &value);
	if(status)
		return status;

	value &= ~BIT_PWR_MGMT_0_S4S_MASK;
	value |= new_value;

	status = inv_icm426xx_write_reg(s, MPUREG_PWR_MGMT_0, 1, &value);

	return status;
}

/** @brief Read PWR_MGMT_0 register S4S_EN bit
 *
 *  <pre>
 *  0 – s4s disabled
 *  1 – s4s enabled
 *  </pre>
 * @param[out] value See enum ICM426XX_PWR_MGMT_0_S4S_t
 * @return 0 in case of success, negative value on error. See enum inv_error
 */
int inv_icm426xx_rd_pwr_mgmt0_s4s_en(struct inv_icm426xx * s, ICM426XX_PWR_MGMT_0_S4S_t * value)
{
	int status;

	status = inv_icm426xx_read_reg(s, MPUREG_PWR_MGMT_0, 1, (uint8_t *)value);
	if(status)
		return status;

	*value &= BIT_PWR_MGMT_0_S4S_MASK;

	return status;
}

int inv_icm426xx_rd_sensor_config0_s4s_mode(struct inv_icm426xx * s, ICM426XX_SENSOR_CONFIG0_S4S_MODE_t * value)
{
	int status;
	uint8_t bank;
	
	// Set memory bank 1
	bank = 1;
	status = inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);
	
	status |= inv_icm426xx_read_reg(s, MPUREG_SENSOR_CONFIG0_B1, 1, (uint8_t *)value);
	
	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	*value &= BIT_SENSOR_CONFIG0_S4S_MODE_MASK;

	return status;
}

int inv_icm426xx_wr_sensor_config0_s4s_mode(struct inv_icm426xx * s, ICM426XX_SENSOR_CONFIG0_S4S_MODE_t new_value)
{
	uint8_t value, bank;
	int status;

	// Set memory bank 1
	bank = 1;
	status = inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);
	
	status |= inv_icm426xx_read_reg(s, MPUREG_SENSOR_CONFIG0_B1, 1, &value);

	value &= ~BIT_SENSOR_CONFIG0_S4S_MODE_MASK;
	value |= new_value;

	status |= inv_icm426xx_write_reg(s, MPUREG_SENSOR_CONFIG0_B1, 1, &value);

	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	return status;
}


int inv_icm426xx_rd_sensor_config2_ois_mode(struct inv_icm426xx * s, ICM426XX_SENSOR_CONFIG2_OIS_MODE_t * value)
{
	int status;
	uint8_t bank;
	
	// Set memory bank 1
	bank = 1;
	status = inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);
	
	status |= inv_icm426xx_read_reg(s, MPUREG_SENSOR_CONFIG2_B1, 1, (uint8_t *)value);
	
	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	*value &= BIT_SENSOR_CONFIG2_OIS_MODE_MASK;

	return status;
}
int inv_icm426xx_wr_sensor_config2_ois_mode(struct inv_icm426xx * s, ICM426XX_SENSOR_CONFIG2_OIS_MODE_t new_value)
{
	uint8_t value, bank;
	int status;

	// Set memory bank 1
	bank = 1;
	status = inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);
	
	status |= inv_icm426xx_read_reg(s, MPUREG_SENSOR_CONFIG2_B1, 1, &value);

	value &= ~BIT_SENSOR_CONFIG2_OIS_MODE_MASK;
	value |= new_value;

	status |= inv_icm426xx_write_reg(s, MPUREG_SENSOR_CONFIG2_B1, 1, &value);

	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	return status;
}


int inv_icm426xx_rd_sensor_config2_nospur_mode(struct inv_icm426xx * s, ICM426XX_SENSOR_CONFIG2_NOSPUR_MODE_t * value)
{
	int status;
	uint8_t bank;
	
	// Set memory bank 1
	bank = 1;
	status = inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);
	
	status |= inv_icm426xx_read_reg(s, MPUREG_SENSOR_CONFIG2_B1, 1, (uint8_t *)value);
	
	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	*value &= BIT_SENSOR_CONFIG2_NOSPUR_MODE_MASK;

	return status;
}
int inv_icm426xx_wr_sensor_config2_nospur_mode(struct inv_icm426xx * s, ICM426XX_SENSOR_CONFIG2_NOSPUR_MODE_t new_value)
{
	uint8_t value, bank;
	int status;

	// Set memory bank 1
	bank = 1;
	status = inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);
	
	status |= inv_icm426xx_read_reg(s, MPUREG_SENSOR_CONFIG2_B1, 1, &value);

	value &= ~BIT_SENSOR_CONFIG2_NOSPUR_MODE_MASK;
	value |= new_value;

	status |= inv_icm426xx_write_reg(s, MPUREG_SENSOR_CONFIG2_B1, 1, &value);

	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	return status;
}

int inv_icm426xx_rd_sensor_config3_rtc_feature_disable(struct inv_icm426xx * s, ICM426XX_SENSOR_CONFIG3_RTC_FEATURE_DISABLE_t * value)
{
	int status;
	uint8_t bank;
	
	// Set memory bank 1
	bank = 1;
	status = inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);
	
	status |= inv_icm426xx_read_reg(s, MPUREG_SENSOR_CONFIG3_B1, 1, (uint8_t *)value);
	*value &= BIT_SENSOR_CONFIG3_RTC_FEATURE_DISABLE_MASK;

	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	return status;
}
int inv_icm426xx_wr_sensor_config3_rtc_feature_disable(struct inv_icm426xx * s, ICM426XX_SENSOR_CONFIG3_RTC_FEATURE_DISABLE_t new_value)
{
	uint8_t value, bank;
	int status;

	// Set memory bank 1
	bank = 1;
	status = inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);
	
	status |= inv_icm426xx_read_reg(s, MPUREG_SENSOR_CONFIG3_B1, 1, &value);

	value &= ~BIT_SENSOR_CONFIG3_RTC_FEATURE_DISABLE_MASK;
	value |= new_value;

	status |= inv_icm426xx_write_reg(s, MPUREG_SENSOR_CONFIG3_B1, 1, &value);

	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	return status;
}

int inv_icm426xx_rd_sensor_config3_ois1_feature_disable(struct inv_icm426xx * s, ICM426XX_SENSOR_CONFIG3_OIS1_FEATURE_DISABLE_t * value)
{
	int status;
	uint8_t bank;
	
	// Set memory bank 1
	bank = 1;
	status = inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);
	
	status |= inv_icm426xx_read_reg(s, MPUREG_SENSOR_CONFIG3_B1, 1, (uint8_t *)value);
	*value &= BIT_SENSOR_CONFIG3_OIS1_FEATURE_DISABLE_MASK;

	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	return status;
}
int inv_icm426xx_wr_sensor_config3_ois1_feature_disable(struct inv_icm426xx * s, ICM426XX_SENSOR_CONFIG3_OIS1_FEATURE_DISABLE_t new_value)
{
	uint8_t value, bank;
	int status;

	// Set memory bank 1
	bank = 1;
	status = inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);
	
	status |= inv_icm426xx_read_reg(s, MPUREG_SENSOR_CONFIG3_B1, 1, &value);

	value &= ~BIT_SENSOR_CONFIG3_OIS1_FEATURE_DISABLE_MASK;
	value |= new_value;

	status |= inv_icm426xx_write_reg(s, MPUREG_SENSOR_CONFIG3_B1, 1, &value);

	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	return status;
}

int inv_icm426xx_rd_sensor_config3_ois2_feature_disable(struct inv_icm426xx * s, ICM426XX_SENSOR_CONFIG3_OIS2_FEATURE_DISABLE_t * value)
{
	int status;
	uint8_t bank;
	
	// Set memory bank 1
	bank = 1;
	status = inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);
	
	status |= inv_icm426xx_read_reg(s, MPUREG_SENSOR_CONFIG3_B1, 1, (uint8_t *)value);
	*value &= BIT_SENSOR_CONFIG3_OIS2_FEATURE_DISABLE_MASK;

	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	return status;
}
int inv_icm426xx_wr_sensor_config3_ois2_feature_disable(struct inv_icm426xx * s, ICM426XX_SENSOR_CONFIG3_OIS2_FEATURE_DISABLE_t new_value)
{
	uint8_t value, bank;
	int status;

	// Set memory bank 1
	bank = 1;
	status = inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);
	
	status |= inv_icm426xx_read_reg(s, MPUREG_SENSOR_CONFIG3_B1, 1, &value);

	value &= ~BIT_SENSOR_CONFIG3_OIS2_FEATURE_DISABLE_MASK;
	value |= new_value;

	status |= inv_icm426xx_write_reg(s, MPUREG_SENSOR_CONFIG3_B1, 1, &value);

	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	return status;
}

/** @brief Set SEC_AUTH0 register SEC_AUTH_MODE bit
 *
 *  When set to 1, this bit will disable all sensors but allow the Master Bias and 
 *  RC oscillator to operate. In this mode, the sec authorization protocol can be operated.
 *  Nominally this bit is set to 0
 *
 *  @param[in] new_value See enum ICM426XX_SEC_AUTH0_MODE_t
 *  @return 0 in case of success, negative value on error. See enum inv_error
 */
int inv_icm426xx_wr_sec_auth0_mode(struct inv_icm426xx * s, ICM426XX_SEC_AUTH0_MODE_t new_value)
{
	uint8_t value, bank;
	int status = 0;

	// Set memory bank 1
	bank = 1;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);
	
	status |= inv_icm426xx_read_reg(s, MPUREG_SEC_AUTH0_B1, 1, &value);

	value &= ~BIT_SEC_AUTH0_MODE_MASK;
	value |= new_value;

	status |= inv_icm426xx_write_reg(s, MPUREG_SEC_AUTH0_B1, 1, &value);

	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	return status;
}

/** @brief Set SEC_START register SEC_AUTH_MODE bit
 *
 *  Starts secure authentication procedure
 *
 *  @param[in] new_value See enum ICM426XX_SEC_AUTH0_START_t
 *  @return 0 in case of success, negative value on error. See enum inv_error
 */
int inv_icm426xx_wr_sec_auth0_start(struct inv_icm426xx * s, ICM426XX_SEC_AUTH0_START_t new_value)
{
	uint8_t value, bank;
	int status = 0;

	// Set memory bank 1
	bank = 1;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);
	
	status |= inv_icm426xx_read_reg(s, MPUREG_SEC_AUTH0_B1, 1, &value);

	value &= ~BIT_SEC_AUTH0_START_MASK;
	value |= new_value;

	status |= inv_icm426xx_write_reg(s, MPUREG_SEC_AUTH0_B1, 1, &value);

	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	return status;
}


/** @brief Get LOWG_PEAK_TH_HYST
 *
 *  @param[out] value See enum ICM426XX_APEX_CONFIG4_LOWG_PEAK_TH_HYST_T
 *  @return 0 in case of success, negative value on error. See enum inv_error
 */
int inv_icm426xx_rd_apex_config4_lowg_peak_th_hyst(struct inv_icm426xx * s, ICM426XX_APEX_CONFIG4_LOWG_PEAK_TH_HYST_t * value)
{
	uint8_t bank;
	int status;

	// Set memory bank 4
	bank = 4;
	status = inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	status |= inv_icm426xx_read_reg(s, MPUREG_APEX_CONFIG4_B4, 1, (uint8_t *)value);

	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	*value &= BIT_APEX_CONFIG4_LOWG_PEAK_TH_HYST_MASK;

	return status;
}


/** @brief Set LOWG_PEAK_TH_HYST
 *
 *  @param[in] new_value See enum ICM426XX_APEX_CONFIG4_LOWG_PEAK_TH_HYST_T
 *  @return 0 in case of success, negative value on error. See enum inv_error
 */
int inv_icm426xx_wr_apex_config4_lowg_peak_th_hyst(struct inv_icm426xx * s, ICM426XX_APEX_CONFIG4_LOWG_PEAK_TH_HYST_t new_value)
{
	uint8_t value, bank;
	int status;

	// Set memory bank 4
	bank = 4;
	status = inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);
	
	status |= inv_icm426xx_read_reg(s, MPUREG_APEX_CONFIG4_B4, 1, &value);

	value &= ~BIT_APEX_CONFIG4_LOWG_PEAK_TH_HYST_MASK;
	value |= new_value;

	status |= inv_icm426xx_write_reg(s, MPUREG_APEX_CONFIG4_B4, 1, &value);

	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	return status;
}


/** @brief Get HIGHG_PEAK_TH_HYST
 *
 *  @param[out] value See enum ICM426XX_APEX_CONFIG4_HIGHG_PEAK_TH_HYST_T
 *  @return 0 in case of success, negative value on error. See enum inv_error
 */
int inv_icm426xx_rd_apex_config4_highg_peak_th_hyst(struct inv_icm426xx * s, ICM426XX_APEX_CONFIG4_HIGHG_PEAK_TH_HYST_t * value)
{
	uint8_t bank;
	int status;

	// Set memory bank 4
	bank = 4;
	status = inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	status |= inv_icm426xx_read_reg(s, MPUREG_APEX_CONFIG4_B4, 1, (uint8_t *)value);

	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	*value &= BIT_APEX_CONFIG4_HIGHG_PEAK_TH_HYST_MASK;

	return status;
}


/** @brief Set HIGHG_PEAK_TH_HYST
 *
 *  @param[in] new_value See enum ICM426XX_APEX_CONFIG4_HIGHG_PEAK_TH_HYST_T
 *  @return 0 in case of success, negative value on error. See enum inv_error
 */
int inv_icm426xx_wr_apex_config4_highg_peak_th_hyst(struct inv_icm426xx * s, ICM426XX_APEX_CONFIG4_HIGHG_PEAK_TH_HYST_t new_value)
{
	uint8_t value, bank;
	int status;

	// Set memory bank 4
	bank = 4;
	status = inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);
	
	status |= inv_icm426xx_read_reg(s, MPUREG_APEX_CONFIG4_B4, 1, &value);

	value &= ~BIT_APEX_CONFIG4_HIGHG_PEAK_TH_HYST_MASK;
	value |= new_value;

	status |= inv_icm426xx_write_reg(s, MPUREG_APEX_CONFIG4_B4, 1, &value);

	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	return status;
}

/** @brief Get LOWG_PEAK_TH
 *
 *  @param[out] value See enum ICM426XX_APEX_CONFIG5_LOWG_PEAK_TH_t
 *  @return 0 in case of success, negative value on error. See enum inv_error
 */
int inv_icm426xx_rd_apex_config5_lowg_peak_th(struct inv_icm426xx * s, ICM426XX_APEX_CONFIG5_LOWG_PEAK_TH_t * value)
{
	uint8_t bank;
	int status;

	// Set memory bank 4
	bank = 4;
	status = inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	status |= inv_icm426xx_read_reg(s, MPUREG_APEX_CONFIG5_B4, 1, (uint8_t *)value);

	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	*value &= BIT_APEX_CONFIG5_LOWG_PEAK_TH_MASK;

	return status;
}

/** @brief Set LOWG_PEAK_TH
 *
 *  @param[in] new_value See enum ICM426XX_APEX_CONFIG5_LOWG_PEAK_TH_t
 *  @return 0 in case of success, negative value on error. See enum inv_error
 */
int inv_icm426xx_wr_apex_config5_lowg_peak_th(struct inv_icm426xx * s, ICM426XX_APEX_CONFIG5_LOWG_PEAK_TH_t new_value)
{
	uint8_t value, bank;
	int status;

	// Set memory bank 4
	bank = 4;
	status = inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);
	
	status |= inv_icm426xx_read_reg(s, MPUREG_APEX_CONFIG5_B4, 1, &value);

	value &= ~BIT_APEX_CONFIG5_LOWG_PEAK_TH_MASK;
	value |= new_value;

	status |= inv_icm426xx_write_reg(s, MPUREG_APEX_CONFIG5_B4, 1, &value);

	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	return status;
}

/** @brief Get LOWG_TIME_TH
 *
 *  @param[out] value See enum ICM426XX_APEX_CONFIG5_LOWG_TIME_TH_T
 *  @return 0 in case of success, negative value on error. See enum inv_error
 */
int inv_icm426xx_rd_apex_config5_lowg_time_th(struct inv_icm426xx * s, ICM426XX_APEX_CONFIG5_LOWG_TIME_TH_t * value)
{
	uint8_t bank;
	int status;

	// Set memory bank 4
	bank = 4;
	status = inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	status |= inv_icm426xx_read_reg(s, MPUREG_APEX_CONFIG5_B4, 1, (uint8_t *)value);

	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	*value &= BIT_APEX_CONFIG5_LOWG_TIME_TH_MASK;

	return status;
}

/** @brief Set LOWG_TIME_TH
 *
 *  @param[in] new_value See enum ICM426XX_APEX_CONFIG5_LOWG_TIME_TH_T
 *  @return 0 in case of success, negative value on error. See enum inv_error
 */
int inv_icm426xx_wr_apex_config5_lowg_time_th(struct inv_icm426xx * s, ICM426XX_APEX_CONFIG5_LOWG_TIME_TH_t new_value)
{
	uint8_t value, bank;
	int status;

	// Set memory bank 4
	bank = 4;
	status = inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);
	
	status |= inv_icm426xx_read_reg(s, MPUREG_APEX_CONFIG5_B4, 1, &value);

	value &= ~BIT_APEX_CONFIG5_LOWG_TIME_TH_MASK;
	value |= new_value;

	status |= inv_icm426xx_write_reg(s, MPUREG_APEX_CONFIG5_B4, 1, &value);

	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	return status;
}

/** @brief Get HIGHG_PEAK_TH
 *
 *  @param[out] value See enum ICM426XX_APEX_CONFIG6_HIGHG_PEAK_TH_t 
 *  @return 0 in case of success, negative value on error. See enum inv_error
 */
int inv_icm426xx_rd_apex_config6_highg_peak_th(struct inv_icm426xx * s, ICM426XX_APEX_CONFIG6_HIGHG_PEAK_TH_t * value)
{
	uint8_t bank;
	int status;

	// Set memory bank 4
	bank = 4;
	status = inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	status |= inv_icm426xx_read_reg(s, MPUREG_APEX_CONFIG6_B4, 1, (uint8_t *)value);

	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	*value &= BIT_APEX_CONFIG6_HIGHG_PEAK_TH_MASK;

	return status;
}

/** @brief Set HIGHG_PEAK_TH
 *
 *  @param[in] new_value See enum ICM426XX_APEX_CONFIG6_HIGHG_PEAK_TH_t
 *  @return 0 in case of success, negative value on error. See enum inv_error
 */
int inv_icm426xx_wr_apex_config6_highg_peak_th(struct inv_icm426xx * s, ICM426XX_APEX_CONFIG6_HIGHG_PEAK_TH_t new_value)
{
	uint8_t value, bank;
	int status;

	// Set memory bank 4
	bank = 4;
	status = inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);
	
	status |= inv_icm426xx_read_reg(s, MPUREG_APEX_CONFIG6_B4, 1, &value);

	value &= ~BIT_APEX_CONFIG6_HIGHG_PEAK_TH_MASK;
	value |= new_value;

	status |= inv_icm426xx_write_reg(s, MPUREG_APEX_CONFIG6_B4, 1, &value);

	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	return status;
}



/** @brief Get HIGHG_TIME_TH
 *
 *  @param[in] new_value See enum ICM426XX_APEX_CONFIG6_HIGHG_TIME_TH_t
 *  @return 0 in case of success, negative value on error. See enum inv_error
 */
int inv_icm426xx_rd_apex_config6_highg_time_th(struct inv_icm426xx * s, ICM426XX_APEX_CONFIG6_HIGHG_TIME_TH_t * value)
{
	uint8_t bank;
	int status;

	// Set memory bank 4
	bank = 4;
	status = inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	status |= inv_icm426xx_read_reg(s, MPUREG_APEX_CONFIG6_B4, 1, (uint8_t *)value);

	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	*value &= BIT_APEX_CONFIG6_HIGHG_TIME_TH_MASK;

	return status;
}

/** @brief Set HIGHG_TIME_TH
 *
 *  @param[in] new_value See enum ICM426XX_APEX_CONFIG6_HIGHG_TIME_TH_t
 *  @return 0 in case of success, negative value on error. See enum inv_error
 */
int inv_icm426xx_wr_apex_config6_highg_time_th(struct inv_icm426xx * s, ICM426XX_APEX_CONFIG6_HIGHG_TIME_TH_t new_value)
{
	uint8_t value, bank;
	int status;

	// Set memory bank 4
	bank = 4;
	status = inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);
	
	status |= inv_icm426xx_read_reg(s, MPUREG_APEX_CONFIG6_B4, 1, &value);

	value &= ~BIT_APEX_CONFIG6_HIGHG_TIME_TH_MASK;
	value |= new_value;

	status |= inv_icm426xx_write_reg(s, MPUREG_APEX_CONFIG6_B4, 1, &value);

	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	return status;
}

int inv_icm426xx_wr_pll_lp_trim0_gyro_pll_div_trim_d2a(struct inv_icm426xx * s, uint8_t new_value)
{
	uint8_t value, bank;
	int status;

	// Set memory bank 3
	bank = 3;
	status = inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);
	
	status |= inv_icm426xx_read_reg(s, MPUREG_PLL_LP_TRIM0_B3, 1, &value);

	value &= ~BIT_PLL_LP_TRIM0_GYRO_PLL_DIV_TRIM_D2A_MASK;
	value |= new_value;

	status |= inv_icm426xx_write_reg(s, MPUREG_PLL_LP_TRIM0_B3, 1, &value);

	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	return status;
}

int inv_icm426xx_rd_pll_lp_trim0_gyro_pll_div_trim_d2a(struct inv_icm426xx * s, uint8_t * value)
{
	uint8_t bank;
	int status;

	// Set memory bank 3
	bank = 3;
	status = inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);
	
	status |= inv_icm426xx_read_reg(s, MPUREG_PLL_LP_TRIM0_B3, 1, value);
	*value &= BIT_PLL_LP_TRIM0_GYRO_PLL_DIV_TRIM_D2A_MASK;

	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	return status;
}

/** @brief Set DMD_GCT_TRIM1 register
 *
 *  @param[in] new_value 
 *  @return 0 in case of success, negative value on error. See enum inv_error
 */
int inv_icm426xx_wr_dmd_gct_trim1_gyro_adc_chop_trim_d2a(struct inv_icm426xx * s, ICM426XX_DMD_GCT_TRIM1_GYRO_ADC_CHOP_TRIM_D2A_t new_value)
{
	uint8_t value, bank;
	int status;

	// Set memory bank 3
	bank = 3;
	status = inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);
	
	status |= inv_icm426xx_read_reg(s, MPUREG_DMD_GCT_TRIM1_B3, 1, &value);

	value &= ~((uint8_t)BIT_DMD_GCT_TRIM1_GYRO_ADC_CHOP_TRIM_D2A_MASK);
	value |= new_value;

	status |= inv_icm426xx_write_reg(s, MPUREG_DMD_GCT_TRIM1_B3, 1, &value);

	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	return status;
}

int inv_icm426xx_rd_dmd_gct_trim1_gyro_adc_chop_trim_d2a(struct inv_icm426xx * s, ICM426XX_DMD_GCT_TRIM1_GYRO_ADC_CHOP_TRIM_D2A_t * value)
{
	uint8_t bank;
	int status;

	// Set memory bank 3
	bank = 3;
	status = inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);
	
	status |= inv_icm426xx_read_reg(s, MPUREG_DMD_GCT_TRIM1_B3, 1, (uint8_t *)value);
	*value &= BIT_DMD_GCT_TRIM1_GYRO_ADC_CHOP_TRIM_D2A_MASK;

	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	return status;
}

int inv_icm426xx_wr_accel_xy_trim1_accel_adc_chop_rate_trim_d2a(struct inv_icm426xx * s, ICM426XX_ACCEL_XY_TRIM1_ACCEL_ADC_CHOP_RATE_TRIM_D2A_t new_value)
{
	uint8_t value, bank;
	int status;

	// Set memory bank 3
	bank = 3;
	status = inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);
	
	status |= inv_icm426xx_read_reg(s, MPUREG_ACCEL_XY_TRIM1_B3, 1, &value);

	value &= ~((uint8_t)BIT_ACCEL_XY_TRIM1_ACCEL_ADC_CHOP_RATE_TRIM_D2A_MASK);
	value |= new_value;

	status |= inv_icm426xx_write_reg(s, MPUREG_ACCEL_XY_TRIM1_B3, 1, &value);

	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	return status;
}
int inv_icm426xx_rd_accel_xy_trim1_accel_adc_chop_rate_trim_d2a(struct inv_icm426xx * s, ICM426XX_ACCEL_XY_TRIM1_ACCEL_ADC_CHOP_RATE_TRIM_D2A_t * value)
{
	uint8_t bank;
	int status;

	// Set memory bank 3
	bank = 3;
	status = inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);
	
	status |= inv_icm426xx_read_reg(s, MPUREG_ACCEL_XY_TRIM1_B3, 1, (uint8_t *)value);
	*value &= BIT_ACCEL_XY_TRIM1_ACCEL_ADC_CHOP_RATE_TRIM_D2A_MASK;

	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	return status;
}


int inv_icm426xx_wr_dig_clk_trim2_gyro_dmd_divratio_trim_d2d(struct inv_icm426xx * s, uint8_t new_value)
{
	uint8_t value, bank;
	int status;

	// Set memory bank 3
	bank = 3;
	status = inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);
	
	status |= inv_icm426xx_read_reg(s, MPUREG_DIG_CLK_TRIM2_B3, 1, &value);

	value &= ~BIT_DIG_CLK_TRIM2_GYRO_DMD_DIVRATIO_TRIM_D2D_MASK;
	value |= new_value;

	status |= inv_icm426xx_write_reg(s, MPUREG_DIG_CLK_TRIM2_B3, 1, &value);

	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	return status;
}

int inv_icm426xx_rd_dig_clk_trim2_gyro_dmd_divratio_trim_d2d(struct inv_icm426xx * s, uint8_t * value)
{
	uint8_t bank;
	int status;

	// Set memory bank 3
	bank = 3;
	status = inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);
	
	status |= inv_icm426xx_read_reg(s, MPUREG_DIG_CLK_TRIM2_B3, 1, value);
	*value &= BIT_DIG_CLK_TRIM2_GYRO_DMD_DIVRATIO_TRIM_D2D_MASK;

	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	return status;
}

/** @brief Set S4S_GYRO_TPH1 register
 *
 *  @param[in] new_value 
 *  @return 0 in case of success, negative value on error. See enum inv_error
 */
int inv_icm426xx_wr_s4s_gyro_tph1(struct inv_icm426xx * s, uint8_t new_value)
{
	uint8_t bank;
	int status;

	// Set memory bank 3
	bank = 3;
	status = inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	status |= inv_icm426xx_write_reg(s, MPUREG_S4S_GYRO_TPH1_B3, 1, &new_value);

	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	return status;
}

/** @brief Set S4S_GYRO_TPH2 register
 *
 *  @param[in] new_value 
 *  @return 0 in case of success, negative value on error. See enum inv_error
 */
int inv_icm426xx_wr_s4s_gyro_tph2(struct inv_icm426xx * s, uint8_t new_value)
{
	uint8_t bank;
	int status;

	// Set memory bank 3
	bank = 3;
	status = inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	status |= inv_icm426xx_write_reg(s, MPUREG_S4S_GYRO_TPH2_B3, 1, &new_value);

	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	return status;
}

/** @brief Set S4S_ACCEL_TPH1 register
 *
 *  @param[in] new_value 
 *  @return 0 in case of success, negative value on error. See enum inv_error
 */
int inv_icm426xx_wr_s4s_accel_tph1(struct inv_icm426xx * s, uint8_t new_value)
{
	uint8_t bank;
	int status;

	// Set memory bank 3
	bank = 3;
	status = inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	status |= inv_icm426xx_write_reg(s, MPUREG_S4S_ACCEL_TPH1_B3, 1, &new_value);

	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	return status;
}

/** @brief Set S4S_ACCEL_TPH2 register
 *
 *  @param[in] new_value 
 *  @return 0 in case of success, negative value on error. See enum inv_error
 */
int inv_icm426xx_wr_s4s_accel_tph2(struct inv_icm426xx * s, uint8_t new_value)
{
	uint8_t bank;
	int status;

	// Set memory bank 3
	bank = 3;
	status = inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	status |= inv_icm426xx_write_reg(s, MPUREG_S4S_ACCEL_TPH2_B3, 1, &new_value);

	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	return status;
}

/** @brief Set S4S_RR register
 *
 *  @param[in] new_value. See enum ICM426XX_S4S_RR_t 
 *  @return 0 in case of success, negative value on error. See enum inv_error
 */
int inv_icm426xx_wr_s4s_rr(struct inv_icm426xx * s, ICM426XX_S4S_RR_t new_value)
{
	uint8_t value, bank;
	int status;

	// Set memory bank 3
	bank = 3;
	status = inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);
	
	status |= inv_icm426xx_read_reg(s, MPUREG_S4S_RR_B3, 1, &value);

	value &= ~BIT_S4S_RR_SEL_MASK;
	value |= new_value;

	status |= inv_icm426xx_write_reg(s, MPUREG_S4S_RR_B3, 1, &value);

	// Set memory bank 0
	bank = 0;
	status |= inv_icm426xx_write_reg(s, MPUREG_REG_BANK_SEL, 1, &bank);

	return status;
}

/** @brief Set GYRO_ACCEL_CONFIG0 register GYRO_FILT_AVG bit
 *
 *  <pre>
 *  1 AVG=1 AVG filt at 1kHz/8kHz
 *  2 AVG=2 AVG filt at 1kHz/8kHz
 *  3 AVG=3 AVG filt at 1kHz/8kHz
 *  4 AVG=4 AVG filt at 1kHz/8kHz
 *  5 AVG=8 AVG filt at 1kHz/8kHz
 *  6 AVG=16 AVG filt at 1kHz/8kHz
 *  7 AVG=32 AVG filt at 1kHz/8kHz
 *  8 AVG=64 AVG filt at 1kHz/8kHz
 *  9 AVG=128 AVG filt at 1kHz/8kHz
 *  </pre>
 * @param[in] new_value See enum ICM426XX_GYRO_ACCEL_CONFIG0_GYRO_FILT_AVG_t
 * @return 0 in case of success, negative value on error. See enum inv_error
 */
int inv_icm426xx_wr_gyro_accel_config0_gyro_filt_avg(struct inv_icm426xx * s, ICM426XX_GYRO_ACCEL_CONFIG0_GYRO_FILT_AVG_t new_value)
{
	uint8_t value;
	int status;

	status = inv_icm426xx_read_reg(s, MPUREG_ACCEL_GYRO_CONFIG0, 1, &value);
	if(status)
		return status;

	value &= ~BIT_GYRO_ACCEL_CONFIG0_GYRO_FILT_MASK;
	value |= new_value;

	status = inv_icm426xx_write_reg(s, MPUREG_ACCEL_GYRO_CONFIG0, 1, &value);
	return status;
}

int inv_icm426xx_rd_gyro_accel_config0_gyro_filt_avg(struct inv_icm426xx * s, ICM426XX_GYRO_ACCEL_CONFIG0_GYRO_FILT_AVG_t * value)
{
	int status;

	status = inv_icm426xx_read_reg(s, MPUREG_ACCEL_GYRO_CONFIG0, 1, (uint8_t *)value);
	if(status)
		return status;

	*value &= BIT_GYRO_ACCEL_CONFIG0_GYRO_FILT_MASK;

	return status;
}

/** @brief Set GYRO_CONFIG1 register AVG_FILT_RATE bit
 *
 *  <pre>
 *  1 in LPM Average filter runs 8KHz
 *  0 in LPM Average filter runs 1KHz
 *  </pre>
 * @param[in] new_value See enum ICM426XX_GYRO_CONFIG1_AVG_FILT_RATE_t
 * @return 0 in case of success, negative value on error. See enum inv_error
 */
int inv_icm426xx_wr_gyro_config1_avg_filt_rate(struct inv_icm426xx * s, ICM426XX_GYRO_CONFIG1_AVG_FILT_RATE_t new_value)
{
	uint8_t value;
	int status;

	status = inv_icm426xx_read_reg(s, MPUREG_GYRO_CONFIG1, 1, &value);
	if(status)
		return status;

	value &= ~BIT_GYRO_CONFIG1_AVG_FILT_RATE_MASK;
	value |= new_value;

	status = inv_icm426xx_write_reg(s, MPUREG_GYRO_CONFIG1, 1, &value);
	return status;
}

int inv_icm426xx_rd_gyro_config1_avg_filt_rate(struct inv_icm426xx * s, ICM426XX_GYRO_CONFIG1_AVG_FILT_RATE_t * value)
{
	int status;
	
	status = inv_icm426xx_read_reg(s, MPUREG_GYRO_CONFIG1, 1, (uint8_t *)value);
	if(status)
		return status;

	*value &= BIT_GYRO_CONFIG1_AVG_FILT_RATE_MASK;

	return status;
}

/** @} */
