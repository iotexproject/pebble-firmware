/*
 * 
 * IOTEX Power Manager
 * 
 */

#include <assert.h>
#include <zephyr.h>
#include <drivers/gpio.h>
#include <sys/__assert.h>
#include <nrf9160.h>
#include <stdio.h>
#include <string.h>
#include <drivers/adc.h>
#include <nrfx.h>
#include <nrfx_saadc.h>



#define SAADC_DEFAULT_CHANNEL_CONFIG    \
{                                                   \
    .resistor_p = NRF_SAADC_RESISTOR_DISABLED,      \
    .resistor_n = NRF_SAADC_RESISTOR_DISABLED,      \
    .gain       = NRF_SAADC_GAIN1_6,                \
    .reference  = NRF_SAADC_REFERENCE_INTERNAL,     \
    .acq_time   = NRF_SAADC_ACQTIME_10US,           \
    .mode       = NRF_SAADC_MODE_SINGLE_ENDED,      \
    .burst      = NRF_SAADC_BURST_DISABLED          \
}


#define SAADC_DEFAULT_CHANNEL(PIN_P) \
{                                      \
    .channel_config =   ((nrf_saadc_channel_config_t)SAADC_DEFAULT_CHANNEL_CONFIG),  \
    .pin_p      = (nrf_saadc_input_t)(PIN_P),       \
    .pin_n      = NRF_SAADC_INPUT_DISABLED,          \
    .channel_index = 0                                  \
}

nrfx_saadc_channel_t  saadc_channel_config_1 = SAADC_DEFAULT_CHANNEL(NRF_SAADC_INPUT_AIN0);


#define BUFFER_SIZE 3
static nrf_saadc_value_t m_sample_buffer[BUFFER_SIZE];


extern void PowerOfIndicator(void);

void adc_module_init(void)
{
    nrfx_err_t err_code;
    nrfx_saadc_init(7);
    if(err_code != NRFX_SUCCESS){
        printk("nrfx_saadc_init error : %d\n",err_code);
    }
    nrfx_saadc_channels_config(&saadc_channel_config_1,1);
    nrfx_saadc_simple_mode_set(1,NRF_SAADC_RESOLUTION_10BIT,NRF_SAADC_OVERSAMPLE_DISABLED,NULL);
    nrfx_saadc_buffer_set(m_sample_buffer,1);
}
int power_check(void)
{
    float adc_voltage = 0;
    nrfx_saadc_mode_trigger();   
    adc_voltage = m_sample_buffer[0]/1023.0 * 2 * 3600.0;
    if(adc_voltage < 3300.0)
    {
        PowerOfIndicator();
    }
    return 0;
}

//--------------------------------------------------------Fatal Error ----------------------------
#include <nrfx_timer.h>
static const nrfx_timer_t m_timer2 = NRFX_TIMER_INSTANCE(2);
extern void fatalError();


static void Timer_2_Interrupt_Handler(nrf_timer_event_t event_type, void *p_context)
{
    //nrfx_timer_disable(&m_timer2);
    fatalError();
}

void closeTimer2(void)
{
    nrfx_timer_disable(&m_timer2);
}

void Timer_Initialize(void) {
    nrfx_err_t err_code;
    nrfx_timer_config_t tmr_config = NRFX_TIMER_DEFAULT_CONFIG;
    tmr_config.frequency = (nrf_timer_frequency_t)NRF_TIMER_FREQ_31250Hz;
    tmr_config.mode = (nrf_timer_mode_t)NRF_TIMER_MODE_TIMER;
    tmr_config.bit_width = (nrf_timer_bit_width_t)NRF_TIMER_BIT_WIDTH_16;
                
    IRQ_CONNECT(0x11,0x1, nrfx_isr, nrfx_timer_2_irq_handler, 0); 
    irq_unlock(0);    
    err_code = nrfx_timer_init(&m_timer2, &tmr_config,(nrfx_timer_event_handler_t) Timer_2_Interrupt_Handler);
    nrfx_timer_extended_compare(&m_timer2, NRF_TIMER_CC_CHANNEL3,31250,NRF_TIMER_SHORT_COMPARE3_STOP_MASK, true);
    nrfx_timer_enable(&m_timer2);
}


