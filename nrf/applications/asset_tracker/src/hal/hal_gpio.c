#include <zephyr.h>
#include <drivers/gpio.h>
#include "hal_gpio.h"
#include "ui.h"

#define GPIO_DIR_OUT  GPIO_OUTPUT
#define GPIO_DIR_IN   GPIO_INPUT
#define GPIO_INT  GPIO_INT_ENABLE
#define GPIO_INT_DOUBLE_EDGE  GPIO_INT_EDGE_BOTH
#define gpio_pin_write  gpio_pin_set

struct device *__gpio0_dev;
static u32_t g_key_press_start_time;
static struct gpio_callback chrq_gpio_cb, pwr_key_gpio_cb;

void checkCHRQ(void)
{
    u32_t chrq;    
    chrq = gpio_pin_get(__gpio0_dev, IO_NCHRQ);
    //gpio_pin_write(port, LED_RED, chrq);    
    //gpio_pin_write(port, LED_GREEN, (chrq + 1 ) % 2);
    if(!chrq)
    {// charging
        ui_led_active(BAT_CHARGING_MASK,0);
        gpio_pin_write(__gpio0_dev, LED_RED, 0);
    }
    else
    {// not charging
        ui_led_deactive(BAT_CHARGING_MASK,0);
        gpio_pin_write(__gpio0_dev, LED_RED, 1);
    }    
}

static void chrq_input_callback(struct device *port, struct gpio_callback *cb, u32_t pins) {
    
    printk("Charge pin %d triggered\n", IO_NCHRQ);

    checkCHRQ();
}

static void pwr_key_callback(struct device *port, struct gpio_callback *cb, u32_t pins) {

    u32_t pwr_key, end_time;
    int32_t key_press_duration, ret;

    //if ((ret = gpio_pin_read(port, POWER_KEY, &pwr_key))) {
    //    return;
    //}
    pwr_key = gpio_pin_get(port, POWER_KEY);

    if (IS_KEY_PRESSED(pwr_key)) {
        g_key_press_start_time = k_uptime_get_32();
        printk("Power key pressed:[%u]\n", g_key_press_start_time);
    }
    else {
        end_time = k_uptime_get_32();
        printk("Power key released:[%u]\n", end_time);

        if (end_time > g_key_press_start_time) {
            key_press_duration = end_time - g_key_press_start_time;
        }
        else {
            key_press_duration = end_time + (int32_t)g_key_press_start_time;
        }

        printk("Power key press duration: %d ms\n", key_press_duration);

        if (key_press_duration > KEY_POWER_OFF_TIME) {
            gpio_pin_write(port, IO_POWER_ON, POWER_OFF);
        }
    }
}

void iotex_hal_gpio_init(void) {

    __gpio0_dev = device_get_binding("GPIO_0");

    /* Set LED pin as output */
    gpio_pin_configure(__gpio0_dev, IO_POWER_ON, GPIO_DIR_OUT);	//p0.31 == POWER_ON
    gpio_pin_configure(__gpio0_dev, LED_GREEN, GPIO_DIR_OUT); 	//p0.00 == LED_GREEN
    gpio_pin_configure(__gpio0_dev, LED_BLUE, GPIO_DIR_OUT);	//p0.01 == LED_BLUE
    gpio_pin_configure(__gpio0_dev, LED_RED, GPIO_DIR_OUT); 	//p0.02 == LED_RED

    gpio_pin_write(__gpio0_dev, IO_POWER_ON, POWER_ON);	//p0.31 == POWER_ON
    gpio_pin_write(__gpio0_dev, LED_GREEN, LED_ON);	//p0.00 == LED_GREEN ON
    gpio_pin_write(__gpio0_dev, LED_BLUE, LED_OFF);	//p0.00 == LED_BLUE OFF
    gpio_pin_write(__gpio0_dev, LED_RED, LED_OFF);	//p0.00 == LED_RED

    /* USB battery charge pin */
    gpio_pin_configure(__gpio0_dev, IO_NCHRQ,
                       (GPIO_DIR_IN | GPIO_INT |
                        GPIO_INT_EDGE | GPIO_INT_DOUBLE_EDGE |
                        GPIO_INT_DEBOUNCE));

    gpio_init_callback(&chrq_gpio_cb, chrq_input_callback, BIT(IO_NCHRQ));
    gpio_add_callback(__gpio0_dev, &chrq_gpio_cb);
    //gpio_pin_enable_callback(__gpio0_dev, IO_NCHRQ);
    gpio_pin_interrupt_configure(__gpio0_dev, IO_NCHRQ,GPIO_INT_EDGE_BOTH);
#if 0
    /* Power key pin configure */
    gpio_pin_configure(__gpio0_dev, POWER_KEY,
                       (GPIO_DIR_IN | GPIO_INT |
                        GPIO_INT_EDGE | GPIO_INT_DOUBLE_EDGE |
                        GPIO_INT_DEBOUNCE));

    gpio_init_callback(&pwr_key_gpio_cb, pwr_key_callback, BIT(POWER_KEY));
    gpio_add_callback(__gpio0_dev, &pwr_key_gpio_cb);
    gpio_pin_enable_callback(__gpio0_dev, POWER_KEY);
#endif
    /* Sync charge state */
    chrq_input_callback(__gpio0_dev, &chrq_input_callback, IO_NCHRQ);
    checkCHRQ();
}


int iotex_hal_gpio_set(uint32_t pin, uint32_t value) {
    return gpio_pin_write(__gpio0_dev, pin, value);
}


void gpio_poweroff(void)
{
    gpio_pin_write(__gpio0_dev, IO_POWER_ON, POWER_OFF);    
}

void PowerOffIndicator(void)
{
    int  i;
    ui_leds_stop();
    for(i =0;i<3;i++){
        gpio_pin_write(__gpio0_dev, LED_RED, LED_ON);
        k_sleep(K_MSEC(1000));
        gpio_pin_write(__gpio0_dev, LED_RED, LED_OFF);
        k_sleep(K_MSEC(1000));
    }
    gpio_poweroff();
    k_sleep(K_MSEC(5000));
}

