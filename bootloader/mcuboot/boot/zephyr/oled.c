#if(CONFIG_BOOT_IOTEX_BOARD_VERSION==3)
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

#include <nrfx_twim.h>
#include "bootutil/bootutil_log.h"
#include <drivers/gpio.h>
#include "boot_serial/boot_serial.h"
#include "serial_adapter/serial_adapter.h"



#define TWI_SCL_M   22
#define TWI_SDA_M   23
#define I2C_ADDR 0x3c


#define SSD1306_CMD    0
#define SSD1306_DAT    1
#define SSD1306_WIDTH    128
#define SSD1306_HEIGHT  64

uint8_t s_chDispalyBuffer[128][8];

const nrfx_twim_t m_twi_master=
{
    .p_twim       = NRFX_CONCAT_2(NRF_TWIM, 2),             
    .drv_inst_idx = NRFX_CONCAT_3(NRFX_TWIM, 2, _INST_IDX)
};

static uint8_t s_Iotex_logo[] = {
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x80,0xC0,0xC0,0xE0,0xE0,0xF0,0xF8,0xF8,0xFC,0xFC,0xFE,0xFE,0xFF,0xFF,
0xFF,0xFE,0xFE,0xFC,0xFC,0xF8,0xF8,0xF0,0xE0,0xE0,0xC0,0xC0,0x80,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFC,
0xF8,0xF8,0xF0,0xE1,0xE1,0xC3,0x83,0x87,0x0F,0x1F,0x1F,0x3F,0x7F,0x7F,0xFF,0xFF,
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x7F,0x3F,0xFF,0xFF,0xFF,0xFF,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0xF8,0xFC,0xF8,0x00,0x00,0xF0,0xF8,0xFC,0x1C,0x1C,0x1C,
0x1C,0x1C,0x1C,0x1C,0x1C,0xFC,0xF8,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0xF8,0xFC,
0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF8,0xFC,0xDC,0xDC,0xDC,0xDC,0xDC,0xDC,
0xDC,0xDC,0xDC,0x88,0x00,0x00,0x04,0x0C,0x1C,0x38,0x70,0xE0,0xC0,0xC0,0xE0,0x70,
0x38,0x1C,0x0C,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,
0x3F,0x3F,0x1F,0x0F,0x0F,0x87,0xC3,0xC3,0xE1,0xF0,0xF0,0x78,0x7C,0x3C,0x1E,0x1F,
0x0F,0x07,0x07,0x03,0x01,0x01,0x00,0x00,0x00,0x80,0xFF,0xFF,0xFF,0xFF,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1F,0x3F,0x1F,0x00,0x00,0x03,0x07,0x0F,0x0E,0x0E,0x0E,
0x0E,0x0E,0x0E,0x0E,0x0E,0x0F,0x07,0x03,0x00,0x10,0x38,0x38,0x38,0x38,0x3F,0x3F,
0x3F,0x38,0x38,0x38,0x38,0x10,0x00,0x00,0x03,0x07,0x0E,0x0E,0x0E,0x0E,0x0E,0x0E,
0x0E,0x0E,0x07,0x03,0x00,0x00,0x20,0x30,0x38,0x1C,0x0E,0x07,0x03,0x03,0x07,0x0E,
0x1C,0x38,0x30,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x02,0x07,0x07,0x0F,0x0F,0x1F,0x3F,0x3D,0x7C,0x78,0xF8,0xF0,0xF0,0xE0,
0xF0,0xF0,0xF8,0x78,0x7C,0x3E,0x3E,0x1F,0x0F,0x0F,0x07,0x07,0x03,0x01,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,
0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};


nrfx_err_t twi_master_init(void)
{
    nrfx_err_t ret;
    const nrfx_twim_config_t  config=
    {
        .scl          =       TWI_SCL_M,//选择pin
        .sda         =        TWI_SDA_M,
        .frequency   =         NRF_TWIM_FREQ_400K,//通信速率
        .interrupt_priority   = NRFX_TWIM_DEFAULT_CONFIG_IRQ_PRIORITY,//优先
        .hold_bus_uninit =     false
        };

    ret = nrfx_twim_init(&m_twi_master, &config, NULL, NULL);//调用配置驱动，无中断BLOCK方式

    if (NRFX_SUCCESS == ret)
    {
        nrfx_twim_enable(&m_twi_master);//使能总线
    }

    return ret;
}


void ssd1306_write_byte(uint8_t chData, uint8_t chCmd) 
{
    //  uint8_t senddata[2];
    uint8_t chip = I2C_ADDR;
    unsigned int devaddr;
    uint8_t tx_buffer[2];
    if (chCmd) {
        devaddr= 0x40;
    } else {
        devaddr= 0x00;
    }
    tx_buffer[0]=devaddr;
    tx_buffer[1]=chData;	

    const  nrfx_twim_xfer_desc_t    tx_xfer_dec= NRFX_TWIM_XFER_DESC_TX(chip, tx_buffer,2);
    nrfx_twim_xfer(&m_twi_master, &tx_xfer_dec,0); 
}

void ssd1306_display_on(void)
{
    ssd1306_write_byte(0x8D, SSD1306_CMD);  
    ssd1306_write_byte(0x14, SSD1306_CMD);  
    ssd1306_write_byte(0xAF, SSD1306_CMD);  
}
void ssd1306_refresh_gram(void)
{
    uint8_t i, j;
    
    for (i = 0; i < 8; i ++) {  
        ssd1306_write_byte(0xB0 + i, SSD1306_CMD);    
        ssd1306_write_byte(0x00, SSD1306_CMD); 
        ssd1306_write_byte(0x10, SSD1306_CMD);    
        for (j = 0; j < 128; j ++) {
            ssd1306_write_byte(s_chDispalyBuffer[j][i], SSD1306_DAT); 
        }
    }  
}

void ssd1306_clear_screen(uint8_t chFill)  
{ 
    memset(s_chDispalyBuffer,chFill, sizeof(s_chDispalyBuffer));
    ssd1306_refresh_gram();   
}
void ssd1306_refresh_lines(uint8_t start_line, uint8_t stop_line)
{
    uint8_t i, j;
    
    for (i = start_line; i <= stop_line ; i ++) {          
        ssd1306_write_byte(0xB0 + i, SSD1306_CMD);    
        ssd1306_write_byte(0x00, SSD1306_CMD); 
        ssd1306_write_byte(0x10, SSD1306_CMD);    
        for (j = 0; j < SSD1306_WIDTH; j ++) {
            ssd1306_write_byte(s_chDispalyBuffer[j][i], SSD1306_DAT); 
        }
    }  
}

void ssd1306_display_logo(void)
{
    uint8_t i, j;
    
    //clearDisBuf(0, 5);
    for (i = 0; i < 6; i ++) {    
        for (j = 0; j < 128; j ++) {
            s_chDispalyBuffer[j][i]= s_Iotex_logo[j+i*128];           
        }
    }   
    ssd1306_refresh_lines(0,5);
}

void ssd1306_init(void)
{ 
    twi_master_init();
    ssd1306_write_byte(0xAE, SSD1306_CMD);//--turn off oled panel
    ssd1306_write_byte(0x00, SSD1306_CMD);//---set low column address
    ssd1306_write_byte(0x10, SSD1306_CMD);//---set high column address
    ssd1306_write_byte(0x40, SSD1306_CMD);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
    ssd1306_write_byte(0x81, SSD1306_CMD);//--set contrast control register
    ssd1306_write_byte(0xCF, SSD1306_CMD);// Set SEG Output Current Brightness
    ssd1306_write_byte(0xA1, SSD1306_CMD);//--Set SEG/Column Mapping    
    ssd1306_write_byte(0xC0, SSD1306_CMD);//Set COM/Row Scan Direction  
    ssd1306_write_byte(0xA6, SSD1306_CMD);//--set normal display
    ssd1306_write_byte(0xA8, SSD1306_CMD);//--set multiplex ratio(1 to 64)
    ssd1306_write_byte(0x3f, SSD1306_CMD);//--1/64 duty
    ssd1306_write_byte(0xD3, SSD1306_CMD);//-set display offset    Shift Mapping RAM Counter (0x00~0x3F)
    ssd1306_write_byte(0x00, SSD1306_CMD);//- offset 20
    ssd1306_write_byte(0xd5, SSD1306_CMD);//--set display clock divide ratio/oscillator frequency
    ssd1306_write_byte(0x80, SSD1306_CMD);//--set divide ratio, Set Clock as 100 Frames/Sec
    ssd1306_write_byte(0xD9, SSD1306_CMD);//--set pre-charge period
    ssd1306_write_byte(0xF1, SSD1306_CMD);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
    ssd1306_write_byte(0xDA, SSD1306_CMD);//--set com pins hardware configuration
    ssd1306_write_byte(0x12, SSD1306_CMD);
    ssd1306_write_byte(0xDB, SSD1306_CMD);//--set vcomh
    ssd1306_write_byte(0x40, SSD1306_CMD);//Set VCOM Deselect Level
    ssd1306_write_byte(0x20, SSD1306_CMD);//-Set Page Addressing Mode (0x00/0x01/0x02)
    ssd1306_write_byte(0x02, SSD1306_CMD);//
    ssd1306_write_byte(0x8D, SSD1306_CMD);//--set Charge Pump enable/disable
    ssd1306_write_byte(0x14, SSD1306_CMD);//--set(0x10) disable
    ssd1306_write_byte(0xA4, SSD1306_CMD);// Disable Entire Display On (0xa4/0xa5)
    ssd1306_write_byte(0xA6, SSD1306_CMD);// Disable Inverse Display On (0xa6/a7) 
    //ssd1306_write_byte(0xAF, SSD1306_CMD);//--turn on oled panel
    //ssd1306_display_on();
    ssd1306_clear_screen(0); 

    ssd1306_display_logo();
    //ssd1306_display_string(10,20,"up_key_press",16,1);
    //ssd1306_refresh_gram();

    ssd1306_display_on();
}
#endif