#ifndef _IOTEX_HAL_ADC_H
#define _IOTEX_HAL_ADC_H

#include <hal/nrf_saadc.h>

#define ADC_DEVICE_NAME DT_ADC_0_NAME
#define ADC_RESOLUTION 10
#define ADC_GAIN ADC_GAIN_1_6
#define ADC_REFERENCE ADC_REF_INTERNAL
#define ADC_ACQUISITION_TIME ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 10)


#define ADC_1ST_CHANNEL_ID 0
#define ADC_1ST_CHANNEL_INPUT NRF_SAADC_INPUT_AIN0
#define ADC_2ND_CHANNEL_ID 2
#define ADC_2ND_CHANNEL_INPUT NRF_SAADC_INPUT_AIN2


int iotex_hal_adc_init(void);
float  iotex_hal_adc_sample(void);


#endif //_IOTEX_HAL_ADC_H