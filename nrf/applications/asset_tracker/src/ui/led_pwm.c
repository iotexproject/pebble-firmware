/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#include <zephyr.h>
#include <drivers/pwm.h>
#include <string.h>

#include "ui.h"
#include "led_pwm.h"
#include "led_effect.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(ui_led_pwm, CONFIG_UI_LOG_LEVEL);

struct led {
	struct device *pwm_dev;

	size_t id;
	struct led_color color;
	const struct led_effect *effect;
	u16_t effect_step;
	u16_t effect_substep;

	struct k_delayed_work work;
};
static u8_t  patternIndex = 0;
static const u8_t ui_pattern[]={ 
	UI_NGPS_NBAT_NLTE,   // Blink UI_LED_COLOR_BLUE
	UI_NGPS_NBAT_LTE,    // Breathe UI_LED_COLOR_BLUE
	UI_NGPS_BAT_NLTE,    // blink  UI_LED_COLOR_PURPLE
	UI_NGPS_BAT_LTE,     // breathe UI_LED_COLOR_PURPLE
	UI_GPS_NBAT_NLTE,    // blink   UI_LED_COLOR_CYAN
	UI_GPS_NBAT_LTE,    // breathe   UI_LED_COLOR_CYAN
	UI_GPS_BAT_NLTE,      // blink UI_LED_COLOR_WHITE
	UI_GPS_BAT_LTE,      // breathe UI_LED_COLOR_WHITE
};

static const struct led_effect effect[] = {
	[UI_NGPS_NBAT_NLTE]   = LED_EFFECT_LED_BLINK(UI_LED_BLINK_NORMAL,
					UI_LED_COLOR_BLUE),
	[UI_NGPS_NBAT_LTE] = LED_EFFECT_LED_BREATHE(UI_LED_ON_PERIOD_NORMAL,
					UI_LED_OFF_PERIOD_NORMAL,
					UI_LED_COLOR_BLUE),		
	[UI_NGPS_BAT_NLTE]   = LED_EFFECT_LED_BLINK(UI_LED_BLINK_NORMAL,
					UI_LED_COLOR_PURPLE),	
	[UI_NGPS_BAT_LTE] = LED_EFFECT_LED_BREATHE(UI_LED_ON_PERIOD_NORMAL,
					UI_LED_OFF_PERIOD_NORMAL,
					UI_LED_COLOR_PURPLE),	
	[UI_GPS_NBAT_NLTE]   = LED_EFFECT_LED_BLINK(UI_LED_BLINK_NORMAL,
					UI_LED_COLOR_CYAN),	
	[UI_GPS_NBAT_LTE]   =  LED_EFFECT_LED_BREATHE(UI_LED_ON_PERIOD_NORMAL,
					UI_LED_OFF_PERIOD_NORMAL,
					UI_LED_COLOR_CYAN),		
	[UI_GPS_BAT_NLTE]   = LED_EFFECT_LED_BLINK(UI_LED_BLINK_NORMAL,
					UI_LED_COLOR_WHITE),									
	[UI_GPS_BAT_LTE] = LED_EFFECT_LED_BREATHE(UI_LED_ON_PERIOD_NORMAL,
					UI_LED_OFF_PERIOD_NORMAL,
					UI_LED_COLOR_WHITE),																						
	[UI_LTE_DISCONNECTED] = LED_EFFECT_LED_BREATHE(UI_LED_ON_PERIOD_NORMAL,
					UI_LED_OFF_PERIOD_NORMAL,
					UI_LTE_DISCONNECTED_COLOR),
	[UI_LTE_CONNECTING] = LED_EFFECT_LED_BLINK(UI_LED_BLINK_NORMAL,
					UI_LTE_CONNECTING_COLOR),				
	[UI_LTE_CONNECTED] = LED_EFFECT_LED_BREATHE(UI_LED_ON_PERIOD_NORMAL,
					UI_LED_OFF_PERIOD_NORMAL,
					UI_LTE_CONNECTED_COLOR),
	[UI_CLOUD_CONNECTING] = LED_EFFECT_LED_BREATHE(UI_LED_ON_PERIOD_NORMAL,
					UI_LED_OFF_PERIOD_NORMAL,
					UI_CLOUD_CONNECTING_COLOR),
	[UI_CLOUD_CONNECTED] = LED_EFFECT_LED_BREATHE(UI_LED_ON_PERIOD_NORMAL,
					UI_LED_OFF_PERIOD_NORMAL,
					UI_CLOUD_CONNECTED_COLOR),
	[UI_CLOUD_PAIRING] = LED_EFFECT_LED_BREATHE(UI_LED_ON_PERIOD_NORMAL,
					UI_LED_OFF_PERIOD_NORMAL,
					UI_CLOUD_PAIRING_COLOR),
	[UI_ACCEL_CALIBRATING] = LED_EFFECT_LED_BREATHE(UI_LED_ON_PERIOD_NORMAL,
					UI_LED_OFF_PERIOD_NORMAL,
					UI_ACCEL_CALIBRATING_COLOR),
	[UI_LED_ERROR_CLOUD] = LED_EFFECT_LED_BREATHE(UI_LED_ON_PERIOD_ERROR,
					UI_LED_OFF_PERIOD_ERROR,
					UI_LED_ERROR_CLOUD_COLOR),
	[UI_LED_ERROR_BSD_REC] = LED_EFFECT_LED_BREATHE(UI_LED_ON_PERIOD_ERROR,
					UI_LED_OFF_PERIOD_ERROR,
					UI_LED_ERROR_BSD_REC_COLOR),
	[UI_LED_ERROR_BSD_IRREC] = LED_EFFECT_LED_BREATHE(UI_LED_ON_PERIOD_ERROR,
					UI_LED_OFF_PERIOD_ERROR,
					UI_LED_ERROR_BSD_IRREC_COLOR),
	[UI_LED_ERROR_LTE_LC] = LED_EFFECT_LED_BREATHE(UI_LED_ON_PERIOD_ERROR,
					UI_LED_OFF_PERIOD_ERROR,
					UI_LED_ERROR_LTE_LC_COLOR),
	[UI_LED_ERROR_UNKNOWN] = LED_EFFECT_LED_BREATHE(UI_LED_ON_PERIOD_ERROR,
					UI_LED_OFF_PERIOD_ERROR,
					UI_LED_ERROR_UNKNOWN_COLOR),
	[UI_LED_GPS_SEARCHING] = LED_EFFECT_LED_BREATHE(UI_LED_ON_PERIOD_ERROR,
					UI_LED_OFF_PERIOD_ERROR,
					UI_LED_GPS_SEARCHING_COLOR),
	[UI_LED_GPS_BLOCKED] = LED_EFFECT_LED_BREATHE(UI_LED_ON_PERIOD_ERROR,
					UI_LED_OFF_PERIOD_ERROR,
					UI_LED_GPS_BLOCKED_COLOR),
	[UI_LED_GPS_FIX] = LED_EFFECT_LED_BREATHE(UI_LED_ON_PERIOD_ERROR,
					UI_LED_OFF_PERIOD_ERROR,
					UI_LED_GPS_FIX_COLOR),
};

static struct led_effect custom_effect =
	LED_EFFECT_LED_BREATHE(UI_LED_ON_PERIOD_NORMAL,
		UI_LED_OFF_PERIOD_NORMAL,
		LED_NOCOLOR());

static struct led leds;
static const size_t led_pins[3] = {
	CONFIG_UI_LED_RED_PIN,
	CONFIG_UI_LED_GREEN_PIN,
	CONFIG_UI_LED_BLUE_PIN,
};
extern void checkCHRQ(void);
static void pwm_out(struct led *led, struct led_color *color)
{
	for (size_t i = 0; i < ARRAY_SIZE(color->c); i++) {
		pwm_pin_set_usec(led->pwm_dev, led_pins[i],
				 255, (255-color->c[i]), 0);
	}
}

static void pwm_off(struct led *led)
{
	struct led_color nocolor = {0};

	pwm_out(led, &nocolor);
}

static void work_handler(struct k_work *work)
{
	struct led *led = CONTAINER_OF(work, struct led, work);
	const struct led_effect_step *effect_step =
		&leds.effect->steps[leds.effect_step];
	int substeps_left = effect_step->substep_count - leds.effect_substep;
	struct led_color keep_led_on;

	for (size_t i = 0; i < ARRAY_SIZE(leds.color.c); i++) {
		int diff = (effect_step->color.c[i] - leds.color.c[i]) /
			substeps_left;
		leds.color.c[i] += diff;
	}
	if(isMask(BAT_CHARGING_MASK)) {
		keep_led_on.c[0] = UI_LED_MAX;
		keep_led_on.c[1] = leds.color.c[1];
		keep_led_on.c[2] = leds.color.c[2];
		pwm_out(led, &keep_led_on);
	}
	else {
		leds.color.c[0]=0;
		pwm_out(led, &leds.color);
	}		
	leds.effect_substep++;
	if (leds.effect_substep == effect_step->substep_count) {
		leds.effect_substep = 0;
		leds.effect_step++;

		if (leds.effect_step == leds.effect->step_count) {
			if (leds.effect->loop_forever) {
				leds.effect_step = 0;
			}
		} else {
			__ASSERT_NO_MSG(leds.effect->steps[leds.effect_step].substep_count > 0);
		}
	}

	if (leds.effect_step < leds.effect->step_count) {
		s32_t next_delay =
			leds.effect->steps[leds.effect_step].substep_time;

		k_delayed_work_submit(&leds.work, K_MSEC(next_delay));
	}
}

static void led_update(struct led *led)
{
	k_delayed_work_cancel(&led->work);

	led->effect_step = 0;
	led->effect_substep = 0;

	if (!led->effect) {
		LOG_DBG("No effect set");
		return;
	}

	__ASSERT_NO_MSG(led->effect->steps);

	if (led->effect->step_count > 0) {
		s32_t next_delay =
			led->effect->steps[led->effect_step].substep_time;

		k_delayed_work_submit(&led->work, K_MSEC(next_delay));
	} else {
		LOG_DBG("LED effect with no effect");
	}
}
struct device *beep_pwm_dev = NULL;

int ui_leds_init(void)
{
	const char *dev_name = CONFIG_UI_LED_PWM_DEV_NAME;
	int err = 0;

	beep_pwm_dev = device_get_binding("PWM_1");
	leds.pwm_dev = device_get_binding(dev_name);
	leds.id = 0;
	//patternIndex = 0;
	leds.effect = &effect[UI_LTE_DISCONNECTED];

	if (!leds.pwm_dev) {
		LOG_ERR("Could not bind to device %s", dev_name);
		return -ENODEV;
	}

	k_delayed_work_init(&leds.work, work_handler);
	led_update(&leds);

	return err;
}

void ui_leds_start(void)
{
#ifdef CONFIG_DEVICE_POWER_MANAGEMENT
	int err = device_set_power_state(leds.pwm_dev,
						DEVICE_PM_ACTIVE_STATE,
						NULL, NULL);
	if (err) {
		LOG_ERR("PWM enable failed");
	}
#endif
	led_update(&leds);
}

void ui_leds_stop(void)
{
	k_delayed_work_cancel(&leds.work);
#ifdef CONFIG_DEVICE_POWER_MANAGEMENT
	int err = device_set_power_state(leds.pwm_dev,
					 DEVICE_PM_SUSPEND_STATE,
					 NULL, NULL);
	if (err) {
		LOG_ERR("PWM disable failed");
	}
#endif
	pwm_off(&leds);
}

void ui_led_set_effect(enum ui_led_pattern state)
{
	leds.effect = &effect[state];
	led_update(&leds);
}

void ui_led_set_effect_color(u8_t color,int index)
{
	leds.effect->steps->color.c[index]=color;
}

int ui_led_set_rgb(u8_t red, u8_t green, u8_t blue)
{
	struct led_effect effect =
		LED_EFFECT_LED_BREATHE(UI_LED_ON_PERIOD_NORMAL,
			UI_LED_OFF_PERIOD_NORMAL,
			LED_COLOR(red, green, blue));

	memcpy((void *)custom_effect.steps, (void *)effect.steps,
		effect.step_count * sizeof(struct led_effect_step));

	leds.effect = &custom_effect;
	led_update(&leds);

	return 0;
}
void ui_led_active(u8_t mask, u8_t flg)
{
	patternIndex |= mask;
	if(flg)
		ui_led_set_effect(ui_pattern[patternIndex]);
}
void ui_led_deactive(u8_t mask, u8_t flg)
{
	patternIndex &= (~mask);
	if(flg)
		ui_led_set_effect(ui_pattern[patternIndex]);
}
void updateLedPattern(void)
{	
	checkCHRQ();
	patternIndex &= ALL_UI_MASK;
	if(leds.effect != &effect[ui_pattern[patternIndex]])
		ui_led_set_effect(ui_pattern[patternIndex]);
}
bool isMask(u8_t mask)
{
	return (patternIndex&mask) ? true:false;
}
int onBeepMePressed(int ms)
{
	int err = 0;
	if (!beep_pwm_dev) {
		printk("Could not bind to device %s", beep_pwm_dev);
		err = -ENODEV;
		return err;
	}	
	pwm_pin_set_usec(beep_pwm_dev, 11,370, 185, 0);//2.7kHz ==370us  185/370=50% duty

	k_sleep(K_MSEC(ms));  //1.5S delay

	pwm_pin_set_usec(beep_pwm_dev, 11, 0, 0, 0);

	return err;
}
