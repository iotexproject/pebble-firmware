/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#include <zephyr.h>
#include <sys/util.h>
#include <drivers/gps.h>
#include <modem/lte_lc.h>
#include <drivers/gpio.h>
#include <drivers/uart.h>
#include <stdlib.h>


#include "ui.h"
#include "gps_controller.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(gps_control, CONFIG_ASSET_TRACKER_LOG_LEVEL);

#define  GPIO_DIR_OUT  GPIO_OUTPUT
#define  gpio_pin_write gpio_pin_set

// board v2.0  extern gps
#if(EXTERN_GPS)
static  struct device *guart_dev_gps;
static u8_t uart_buf1[128];
static u8_t pos1=0;

static u8_t print_buf1[128];
static bool printit;
const char strGRMC[7]="$GNRMC";

struct gprmc {
    double latitude;
    char lat;
    double longitude;
    char lon;
    double speed;
    double course;
};
typedef struct gprmc gprmc_t;

static void uart_gps_rx_handler(u8_t character)
{
	
		/* Handle special characters. */
	switch (character) {
	case 0x24: /* Backspace. */
                pos1=0;
        default:        
		/* Fall through. */
		uart_buf1[pos1] = character;
		break;
        }
       
	if ((uart_buf1[pos1 - 1] == '\r') && (character == '\n')) {

                uart_buf1[pos1 - 1] ='\n'; //delete '\r',printk  '\n' to '\r'+'\n'
                uart_buf1[pos1 ] =0;
               // printk("get a cmd\n");
#if 1			   
			   if(strncmp(uart_buf1,strGRMC,strlen(strGRMC))==0){
					strcpy(print_buf1,uart_buf1);
					printit= 1 ;				
			   }
#else
                strcpy(print_buf1,uart_buf1);
                printit= true ;
#endif
                pos1=0;
//                uart_irq_rx_disable(guart_dev_gps);
//                printk("%s",print_buf1);
//                uart_irq_rx_enable(guart_dev_gps);
                //printk("get a cmd\n");            
	}
        else
        {
              pos1++;
	}
   return;

}

static void uart_gps_cb(struct device *dev)
{
	u8_t character;

	uart_irq_update(dev);

	if (!uart_irq_rx_ready(dev)) {
		return;
	}

	while (uart_fifo_read(dev, &character, 1)) {
		uart_gps_rx_handler(character);
	}
}

static int getRMC(char *nmea, gprmc_t *loc)
{
    //GNRMC,084852.000,A,2236.9453,N,11408.4790,E,0.53,292.44,141216,,,A*75
    char *p = nmea;
	char buf[20]={0};
    p = strchr(p, ',') + 1;
    p = strchr(p, ',') + 1;
	if(p[0] != 'A')
		return -1;
    p = strchr(p, ',') + 1;
	memcpy(buf,p, strchr(p, ',')-p);
	if(strlen(buf) == 0)
		return -1;
    loc->latitude = atof(buf);
    p = strchr(p, ',') + 1;
    switch (p[0]) {
    case 'N':
        loc->lat = 'N';
        break;
    case 'S':
        loc->lat = 'S';
        break;
    case ',':
        loc->lat = '\0';
        break;
    }

    p = strchr(p, ',') + 1;
	memset(buf,0,sizeof(buf));
	memcpy(buf,p, strchr(p, ',')-p);
    loc->longitude = atof(buf);

    p = strchr(p, ',') + 1;
    switch(p[0]) {
    case 'W':
        loc->lon = 'W';
        break;
    case 'E':
        loc->lon = 'E';
        break;
    case ',':
        loc->lon = '\0';
        break;
    }

    p = strchr(p, ',') + 1;
    loc->speed = atof(p);

    p = strchr(p, ',') + 1;
    loc->course = atof(p);

	return 0;
}

int getGPS(double *lat, double *lon)
{
	gprmc_t rmc;
	int ret = -1;
    int intpart;
    double fractpart; 	
	if(printit){
		if(!getRMC(print_buf1,&rmc)){
		//if(!getRMC("$GNRMC,093153.00,A,3050.68816,N,11448.65818,E,0.074,,220720,,,A,V*12",&rmc)){
            intpart= rmc.latitude/100;
            fractpart=((rmc.latitude/100)-intpart)/0.6; 
			if(rmc.lat == 'S')                     
   				*lat = (intpart+fractpart)*-1;  
			else
				*lat = intpart+fractpart;
			                    
            intpart= rmc.longitude/100;
            fractpart=((rmc.longitude/100)-intpart)/0.6; 
			if(rmc.lon == 'W')                     
   				*lon = (intpart+fractpart)*-1; 
			else	
				*lon = intpart+fractpart; 									
   			ret = 0;						
		}
		printit = 0;
	}
	if(ret)
		ui_led_deactive(GPS_ACTIVE_MASK,0);
	else
		ui_led_active(GPS_ACTIVE_MASK,0);
	return ret;	
}

void exGPSStart(void)
{
	uart_irq_rx_enable(guart_dev_gps);
}

void exGPSStop(void)
{
	uart_irq_rx_disable(guart_dev_gps);
}

void exGPSInit(void)
{
       struct device *dev;
       dev = device_get_binding("GPIO_0");
       gpio_pin_configure(dev, GPS_EN, GPIO_DIR_OUT); 
       gpio_pin_write(dev, GPS_EN, 1);
       guart_dev_gps=device_get_binding(UART_GPS);
       uart_irq_callback_set(guart_dev_gps, uart_gps_cb);
       uart_irq_rx_enable(guart_dev_gps);
}


#else
/* Structure to hold GPS work information */
static struct device *gps_dev;
static atomic_t gps_is_enabled;
static atomic_t gps_is_active;
static struct k_work_q *app_work_q;
static struct k_delayed_work start_work;
static struct k_delayed_work stop_work;
static int gps_reporting_interval_seconds;

static void start(struct k_work *work)
{
	ARG_UNUSED(work);
	int err;
	struct gps_config gps_cfg = {
		.nav_mode = GPS_NAV_MODE_PERIODIC,
		.power_mode = GPS_POWER_MODE_DISABLED,
		.timeout = CONFIG_GPS_CONTROL_FIX_TRY_TIME,
		.interval = CONFIG_GPS_CONTROL_FIX_TRY_TIME +
			gps_reporting_interval_seconds
	};

	if (gps_dev == NULL) {
		LOG_ERR("GPS controller is not initialized properly");
		return;
	}

#ifdef CONFIG_GPS_CONTROL_PSM_ENABLE_ON_START
	LOG_INF("Enabling PSM");

	err = lte_lc_psm_req(true);
	if (err) {
		LOG_ERR("PSM request failed, error: %d", err);
	} else {
		LOG_INF("PSM enabled");
	}
#endif /* CONFIG_GPS_CONTROL_PSM_ENABLE_ON_START */

	err = gps_start(gps_dev, &gps_cfg);
	if (err) {
		LOG_ERR("Failed to enable GPS, error: %d", err);
		return;
	}

	atomic_set(&gps_is_enabled, 1);
	gps_control_set_active(true);
	ui_led_set_pattern(UI_LED_GPS_SEARCHING);

	LOG_INF("GPS started successfully. Searching for satellites ");
	LOG_INF("to get position fix. This may take several minutes.");
	LOG_INF("The device will attempt to get a fix for %d seconds, ",
		CONFIG_GPS_CONTROL_FIX_TRY_TIME);
	LOG_INF("before the GPS is stopped. It's restarted every %d seconds",
		gps_reporting_interval_seconds);
#if !defined(CONFIG_GPS_SIM)
#if IS_ENABLED(CONFIG_GPS_START_ON_MOTION)
	LOG_INF("or as soon as %d seconds later when movement occurs.",
		CONFIG_GPS_CONTROL_FIX_CHECK_INTERVAL);
#endif
#endif
}

static void stop(struct k_work *work)
{
	ARG_UNUSED(work);
	int err;

	if (gps_dev == NULL) {
		LOG_ERR("GPS controller is not initialized");
		return;
	}

#ifdef CONFIG_GPS_CONTROL_PSM_DISABLE_ON_STOP
	LOG_INF("Disabling PSM");

	err = lte_lc_psm_req(false);
	if (err) {
		LOG_ERR("PSM mode could not be disabled, error: %d",
			err);
	}
#endif /* CONFIG_GPS_CONTROL_PSM_DISABLE_ON_STOP */

	err = gps_stop(gps_dev);
	if (err) {
		LOG_ERR("Failed to disable GPS, error: %d", err);
		return;
	}

	atomic_set(&gps_is_enabled, 0);
	gps_control_set_active(false);
	LOG_INF("GPS operation was stopped");
}

bool gps_control_is_enabled(void)
{
	return atomic_get(&gps_is_enabled);
}

void gps_control_enable(void)
{
#if !defined(CONFIG_GPS_SIM)
	atomic_set(&gps_is_enabled, 1);
	gps_control_start(1000);
#endif
}

void gps_control_disable(void)
{
#if !defined(CONFIG_GPS_SIM)
	atomic_set(&gps_is_enabled, 0);
	gps_control_stop(0);
	ui_led_set_pattern(UI_CLOUD_CONNECTED);
#endif
}
bool gps_control_is_active(void)
{
	return atomic_get(&gps_is_active);
}

bool gps_control_set_active(bool active)
{
	return atomic_set(&gps_is_active, active ? 1 : 0);
}

void gps_control_start(u32_t delay_ms)
{
	k_delayed_work_submit_to_queue(app_work_q, &start_work,
				       K_MSEC(delay_ms));
}

void gps_control_stop(u32_t delay_ms)
{
	k_delayed_work_submit_to_queue(app_work_q, &stop_work,
				       K_MSEC(delay_ms));
}

int gps_control_get_gps_reporting_interval(void)
{
	return gps_reporting_interval_seconds;
}

/** @brief Configures and starts the GPS device. */
int gps_control_init(struct k_work_q *work_q, gps_event_handler_t handler)
{
	int err;
	static bool is_init;

	if (is_init) {
		return -EALREADY;
	}

	if ((work_q == NULL) || (handler == NULL)) {
		return -EINVAL;
	}

	app_work_q = work_q;

	gps_dev = device_get_binding(CONFIG_GPS_DEV_NAME);
	if (gps_dev == NULL) {
		LOG_ERR("Could not get %s device",
			log_strdup(CONFIG_GPS_DEV_NAME));
		return -ENODEV;
	}

	err = gps_init(gps_dev, handler);
	if (err) {
		LOG_ERR("Could not initialize GPS, error: %d", err);
		return err;
	}

	k_delayed_work_init(&start_work, start);
	k_delayed_work_init(&stop_work, stop);

#if !defined(CONFIG_GPS_SIM)
	gps_reporting_interval_seconds =
		IS_ENABLED(CONFIG_GPS_START_ON_MOTION) ?
		CONFIG_GPS_CONTROL_FIX_CHECK_OVERDUE :
		CONFIG_GPS_CONTROL_FIX_CHECK_INTERVAL;
#else
	gps_reporting_interval_seconds = CONFIG_GPS_CONTROL_FIX_CHECK_INTERVAL;
#endif

	LOG_INF("GPS initialized");

	is_init = true;

	return err;
}
#endif
