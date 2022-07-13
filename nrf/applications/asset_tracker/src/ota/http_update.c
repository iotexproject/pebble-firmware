/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */
#include <zephyr.h>
#include <drivers/gpio.h>
#include <drivers/flash.h>
#include <bsd.h>
#include <modem/lte_lc.h>
#include <modem/at_cmd.h>
#include <modem/at_notif.h>
#include <modem/bsdlib.h>
#include <modem/modem_key_mgmt.h>
#include <net/fota_download.h>
#include <dfu/mcuboot.h>

//#define LED_PORT	DT_GPIO_LABEL(DT_ALIAS(led0), gpios)
#define  LED_PORT   "GPIO_0"
#define TLS_SEC_TAG 42

#define LED_GREEN     26         //p0.00 == LED_GREEN  0=on 1=off
#define LED_BLUE      27    //p0.01 == LED_BLUE   0=on 1=off
#define LED_RED       30    //p0.02 == LED_RED    0=on 1=off

#define LED_ON        0
#define LED_OFF       1

#define GPIO_DIR_OUT  GPIO_OUTPUT
#define GPIO_DIR_IN   GPIO_INPUT
#define GPIO_INT  GPIO_INT_ENABLE
#define GPIO_INT_DOUBLE_EDGE  GPIO_INT_EDGE_BOTH
#define gpio_pin_write  gpio_pin_set

static const struct	device *gpiob;
static struct		gpio_callback gpio_cb;
static struct k_work	fota_work;
static struct k_delayed_work	fota_status_check;
static int aliveCnt = 0;

const struct device *dev;

/**@brief Recoverable BSD library error. 
void bsd_recoverable_error_handler(uint32_t err)
{
	printk("bsdlib recoverable error: %u\n", err);
}
*/
int cert_provision(void)
{
	static const char cert[] = {
//		#include "../cert/BaltimoreCyberTrustRoot"
	};
	BUILD_ASSERT(sizeof(cert) < KB(4), "Certificate too large");
	int err;
	bool exists;
	uint8_t unused;

	err = modem_key_mgmt_exists(TLS_SEC_TAG,
				    MODEM_KEY_MGMT_CRED_TYPE_CA_CHAIN,
				    &exists, &unused);
	if (err) {
		printk("Failed to check for certificates err %d\n", err);
		return err;
	}

	if (exists) {
		/* For the sake of simplicity we delete what is provisioned
		 * with our security tag and reprovision our certificate.
		 */
		err = modem_key_mgmt_delete(TLS_SEC_TAG,
					    MODEM_KEY_MGMT_CRED_TYPE_CA_CHAIN);
		if (err) {
			printk("Failed to delete existing certificate, err %d\n",
			       err);
		}
	}

	printk("Provisioning certificate\n");

	/*  Provision certificate to the modem */
	err = modem_key_mgmt_write(TLS_SEC_TAG,
				   MODEM_KEY_MGMT_CRED_TYPE_CA_CHAIN,
				   cert, sizeof(cert) - 1);
	if (err) {
		printk("Failed to provision certificate, err %d\n", err);
		return err;
	}

	return 0;
}
static void show_led(int r, int g, int b)
{

    gpio_pin_write(dev, LED_GREEN, g);	//p0.00 == LED_GREEN ON
    gpio_pin_write(dev, LED_BLUE, b);	//p0.00 == LED_BLUE OFF
	gpio_pin_write(dev, LED_RED, r);	
}
/**@brief Start transfer of the file. */
static void app_dfu_transfer_start(struct k_work *unused)
{
	int retval;
	int sec_tag;
	char *apn = NULL;

#ifndef CONFIG_USE_HTTPS
	sec_tag = -1;
#else
	sec_tag = TLS_SEC_TAG;
#endif
	show_led(LED_OFF,LED_ON,LED_OFF);
	retval = fota_download_start(CONFIG_DOWNLOAD_HOST,
				     CONFIG_DOWNLOAD_FILE,
				     sec_tag,
				     apn,
				     0);
	if (retval != 0) {
		/* Re-enable button callback */
		gpio_pin_interrupt_configure(gpiob,
					     DT_GPIO_PIN(DT_ALIAS(sw0), gpios),
					     GPIO_INT_EDGE_TO_ACTIVE);

		show_led(LED_ON,LED_OFF,LED_OFF);
		printk("fota_download_start() failed, err %d\n",
			retval);
		sys_reboot(0);
	}	
}

static void fota_status(struct k_work *unused)
{
	aliveCnt++;
	if(aliveCnt > 12){
		aliveCnt=0;
		printk("Received hangup from fota_download\n");
		gpio_pin_interrupt_configure(gpiob,
					     DT_GPIO_PIN(DT_ALIAS(sw0), gpios),
					     GPIO_INT_EDGE_TO_ACTIVE);		
		show_led(LED_ON,LED_OFF,LED_OFF);		
		
	}
	else
	{
		k_delayed_work_submit(&fota_status_check, K_MSEC(10000));
	}
	printk("aliveCnt:%d\n", aliveCnt);	
}
/**@brief Turn on LED0 and LED1 if CONFIG_APPLICATION_VERSION
 * is 2 and LED0 otherwise.
 */
static int led_app_version(void)
{
//	const struct device *dev;

	dev = device_get_binding(LED_PORT);
	if (dev == 0) {
		printk("Nordic nRF GPIO driver was not found!\n");
		return 1;
	}

//	gpio_pin_configure(dev, DT_GPIO_PIN(DT_ALIAS(led0), gpios),
//			   GPIO_OUTPUT_ACTIVE |
//			   DT_GPIO_FLAGS(DT_ALIAS(led0), gpios));

    gpio_pin_configure(dev, LED_GREEN, GPIO_DIR_OUT); 	//p0.00 == LED_GREEN
    gpio_pin_configure(dev, LED_BLUE, GPIO_DIR_OUT);	//p0.01 == LED_BLUE
    gpio_pin_configure(dev, LED_RED, GPIO_DIR_OUT); 	//p0.02 == LED_RED

    gpio_pin_write(dev, LED_GREEN, LED_ON);	//p0.00 == LED_GREEN ON
    gpio_pin_write(dev, LED_BLUE, LED_OFF);	//p0.00 == LED_BLUE OFF
	gpio_pin_write(dev, LED_RED, LED_ON);

#if CONFIG_APPLICATION_VERSION == 2
	//gpio_pin_configure(dev, DT_GPIO_PIN(DT_ALIAS(led1), gpios),
	//		   GPIO_OUTPUT_ACTIVE |
	//		   DT_GPIO_FLAGS(DT_ALIAS(led1), gpios));
	gpio_pin_write(dev, LED_RED, LED_ON);
#endif
	return 0;
}


void dfu_button_pressed(const struct device *gpiob, struct gpio_callback *cb,
			uint32_t pins)
{
	k_work_submit(&fota_work);
	k_delayed_work_submit(&fota_status_check, K_MSEC(2000));
	gpio_pin_interrupt_configure(gpiob, DT_GPIO_PIN(DT_ALIAS(sw0), gpios),
				     GPIO_INT_DISABLE);
}

static int dfu_button_init(void)
{
	int err;
/*
	gpiob = device_get_binding(DT_GPIO_LABEL(DT_ALIAS(sw0), gpios));
	if (gpiob == 0) {
		printk("Nordic nRF GPIO driver was not found!\n");
		return 1;
	}
	err = gpio_pin_configure(gpiob, DT_GPIO_PIN(DT_ALIAS(sw0), gpios),
				 GPIO_INPUT |
				 DT_GPIO_FLAGS(DT_ALIAS(sw0), gpios));
	if (err == 0) {
*/        
		gpio_init_callback(&gpio_cb, dfu_button_pressed,
			BIT(DT_GPIO_PIN(DT_ALIAS(sw0), gpios)));
		err = gpio_add_callback(gpiob, &gpio_cb);
//	}
	if (err == 0) {
		err = gpio_pin_interrupt_configure(gpiob,
						   DT_GPIO_PIN(DT_ALIAS(sw0),
							       gpios),
						   GPIO_INT_EDGE_TO_ACTIVE);
	}
	if (err != 0) {
		printk("Unable to configure SW0 GPIO pin!\n");
		return 1;
	}
	return 0;
}
static int initSw0(void)
{
    	int err;

	gpiob = device_get_binding(DT_GPIO_LABEL(DT_ALIAS(sw0), gpios));
	if (gpiob == 0) {
		printk("Nordic nRF GPIO driver was not found!\n");
		return 1;
	}
	err = gpio_pin_configure(gpiob, DT_GPIO_PIN(DT_ALIAS(sw0), gpios),
				 GPIO_INPUT |
				 DT_GPIO_FLAGS(DT_ALIAS(sw0), gpios));
    if(err != 0){        
        return -1;
    }
    else
        return 0;
}
static int  readSw0(void)
{
    u32_t io_lev;
    io_lev = gpio_pin_get(gpiob, DT_GPIO_PIN(DT_ALIAS(sw0), gpios));

	return 0;

    if(io_lev)
    {
        k_sleep(K_MSEC(5000));
        io_lev = gpio_pin_get(gpiob, DT_GPIO_PIN(DT_ALIAS(sw0), gpios));
        if(io_lev)
            return 0;
    }
    return 1;
}

void fota_dl_handler(const struct fota_download_evt *evt)
{
	switch (evt->id) {
	case FOTA_DOWNLOAD_EVT_ERROR:
		printk("Received error from fota_download\n");
		gpio_pin_interrupt_configure(gpiob,
					     DT_GPIO_PIN(DT_ALIAS(sw0), gpios),
					     GPIO_INT_EDGE_TO_ACTIVE);		
		show_led(LED_ON,LED_OFF,LED_OFF);
		break;
		/* Fallthrough */
	case FOTA_DOWNLOAD_EVT_ERASE_PENDING:
		printk("Received timeout from fota_download\n");
		gpio_pin_interrupt_configure(gpiob,
					     DT_GPIO_PIN(DT_ALIAS(sw0), gpios),
					     GPIO_INT_EDGE_TO_ACTIVE);		
		show_led(LED_ON,LED_OFF,LED_OFF);
		break;
	case FOTA_DOWNLOAD_EVT_FINISHED:
		/* Re-enable button callback */
		k_delayed_work_cancel(&fota_status_check);
		gpio_pin_interrupt_configure(gpiob,
					     DT_GPIO_PIN(DT_ALIAS(sw0), gpios),
					     GPIO_INT_EDGE_TO_ACTIVE);
		show_led(LED_ON,LED_ON,LED_ON);
		break;
	case FOTA_DOWNLOAD_EVT_PROGRESS:
		//printk("fota alive \n");
		aliveCnt=0;
		break;

	default:
		break;
	}
}

/**@brief Configures modem to provide LTE link.
 *
 * Blocks until link is successfully established.
 */
static void modem_configure(void)
{
#if defined(CONFIG_LTE_LINK_CONTROL)
	BUILD_ASSERT(!IS_ENABLED(CONFIG_LTE_AUTO_INIT_AND_CONNECT),
			"This sample does not support auto init and connect");
	int err;
#if !defined(CONFIG_BSD_LIBRARY_SYS_INIT)
	/* Initialize AT only if bsdlib_init() is manually
	 * called by the main application
	 */
	err = at_notif_init();
	__ASSERT(err == 0, "AT Notify could not be initialized.");
	err = at_cmd_init();
	__ASSERT(err == 0, "AT CMD could not be established.");
#if defined(CONFIG_USE_HTTPS)
	err = cert_provision();
	__ASSERT(err == 0, "Could not provision root CA to %d", TLS_SEC_TAG);
#endif
#endif
	printk("LTE Link Connecting ...\n");
	err = lte_lc_init_and_connect();
	__ASSERT(err == 0, "LTE link could not be established.");
	printk("LTE Link Connected!\n");
#endif
}

static int application_init(void)
{
	int err;

	k_work_init(&fota_work, app_dfu_transfer_start);
	k_delayed_work_init(&fota_status_check, fota_status);

	err = dfu_button_init();
	if (err != 0) {
		return err;
	}

	err = led_app_version();
	if (err != 0) {
		return err;
	}

	err = fota_download_init(fota_dl_handler);
	if (err != 0) {
		return err;
	}

	return 0;
}

void ota_update(void)
{
	int err;

    if(initSw0()||readSw0())
        return;

	printk("HTTP application update sample started\n");
	printk("Initializing bsdlib\n");
#if !defined(CONFIG_BSD_LIBRARY_SYS_INIT)
	err = bsdlib_init();
#else
	/* If bsdlib is initialized on post-kernel we should
	 * fetch the returned error code instead of bsdlib_init
	 */
	err = bsdlib_get_init_ret();
#endif
	switch (err) {
	case MODEM_DFU_RESULT_OK:
		printk("Modem firmware update successful!\n");
		printk("Modem will run the new firmware after reboot\n");
		k_thread_suspend(k_current_get());
		break;
	case MODEM_DFU_RESULT_UUID_ERROR:
	case MODEM_DFU_RESULT_AUTH_ERROR:
		printk("Modem firmware update failed\n");
		printk("Modem will run non-updated firmware on reboot.\n");
		break;
	case MODEM_DFU_RESULT_HARDWARE_ERROR:
	case MODEM_DFU_RESULT_INTERNAL_ERROR:
		printk("Modem firmware update failed\n");
		printk("Fatal error.\n");
		break;
	case -1:
		printk("Could not initialize bsdlib.\n");
		printk("Fatal error.\n");
		return;
	default:
		break;
	}
	printk("Initialized bsdlib\n");

	modem_configure();
	boot_write_img_confirmed();
	err = application_init();
	if (err != 0) {
		return;
	}

	printk("Press Button 1 to start the FOTA download\n");
	show_led(LED_OFF,LED_OFF,LED_ON);
    while(1)
    {
        k_sleep(K_MSEC(100000));
    }
}
