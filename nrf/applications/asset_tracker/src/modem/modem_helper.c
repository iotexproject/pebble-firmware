#include <stdlib.h>
#include <stdio.h>
#include <zephyr.h>
#include <modem/at_cmd.h>
#include <string.h>
#include "modem_helper.h"


static bool isLeap(uint32_t year) {
    return (((year % 4) == 0) && (((year % 100) != 0) || ((year % 400) == 0)));
}

const char *iotex_modem_get_imei() {

    int err;
    enum at_cmd_state at_state;
    static char imei_buf[MODEM_IMEI_LEN + 5];

    if ((err = at_cmd_write("AT+CGSN", imei_buf, sizeof(imei_buf), &at_state))) {
        printk("Error when trying to do at_cmd_write: %d, at_state: %d", err, at_state);
        return "Unknown";
    }

    return imei_buf;
}

int iotex_model_get_signal_quality() {

    enum at_cmd_state at_state;
    char snr_ack[32], snr[4];
    char *p=snr_ack;

    int err = at_cmd_write("AT+CESQ", snr_ack, 32, &at_state);

    if (err) {
        printk("Error when trying to do at_cmd_write: %d, at_state: %d", err, at_state);
    }
    p = strchr(p, ',') + 1;
    p = strchr(p, ',') + 1;
    p = strchr(p, ',') + 1;
    p = strchr(p, ',') + 1;
    p = strchr(p, ',') + 1;
    //snprintf(snr, sizeof(snr), "%s", &snr_ack[25]);
    snr[0]=p[0];
    snr[1]=p[1];  
    snr[2]=0 ;
    return atoi(snr);
}


const char *iotex_modem_get_clock(iotex_st_timestamp *stamp) {

    double epoch;
    enum at_cmd_state at_state;
    uint32_t YY, MM, DD, hh, mm, ss;

    char cclk_r_buf[TIMESTAMP_STR_LEN];
    static char epoch_buf[TIMESTAMP_STR_LEN];
    int daysPerMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    int err = at_cmd_write("AT+CCLK?", cclk_r_buf, sizeof(cclk_r_buf), &at_state);

    if (err) {
        printk("Error when trying to do at_cmd_write: %d, at_state: %d", err, at_state);
    }

    printk("AT CMD Modem time is:%s\n", cclk_r_buf);

    // num of years since 1900, the formula works only for 2xxx
    YY = (cclk_r_buf[8] - '0') * 10 + (cclk_r_buf[9] - '0') + 100;
    MM = (cclk_r_buf[11] - '0') * 10 + (cclk_r_buf[12] - '0');
    DD = (cclk_r_buf[14] - '0') * 10 + (cclk_r_buf[15] - '0');
    hh = (cclk_r_buf[17] - '0') * 10 + (cclk_r_buf[18] - '0');
    mm = (cclk_r_buf[20] - '0') * 10 + (cclk_r_buf[21] - '0');
    ss = (cclk_r_buf[23] - '0') * 10 + (cclk_r_buf[24] - '0');

    if (isLeap(YY + 1900)) {
        daysPerMonth[2] = 29;
    }

    // accumulate
    for (int i = 1; i <= 12; i++) {
        daysPerMonth[i] += daysPerMonth[i - 1];
    }

    epoch  = ss + mm * 60 + hh * 3600 + (daysPerMonth[ MM - 1] + DD - 1) * 86400 +
             (YY - 70) * 31536000L + ((YY - 69) / 4) * 86400L -
             ((YY - 1) / 100) * 86400L + ((YY + 299) / 400) * 86400L;

    snprintf(epoch_buf, sizeof(epoch_buf), "%.0f", epoch);
    printk("UTC epoch %s\n", epoch_buf);

    if (stamp) {
        snprintf(stamp->data, sizeof(stamp->data), "%.0f", epoch);
        return stamp->data;
    }

    return epoch_buf;
}
double iotex_modem_get_clock_raw(iotex_st_timestamp *stamp) {

    double epoch;
    enum at_cmd_state at_state;
    uint32_t YY, MM, DD, hh, mm, ss;

    char cclk_r_buf[TIMESTAMP_STR_LEN];
    char epoch_buf[TIMESTAMP_STR_LEN];
    int daysPerMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    int err = at_cmd_write("AT+CCLK?", cclk_r_buf, sizeof(cclk_r_buf), &at_state);

    if (err) {
        printk("Error when trying to do at_cmd_write: %d, at_state: %d", err, at_state);
    }

    printk("AT CMD Modem time is:%s\n", cclk_r_buf);

    // num of years since 1900, the formula works only for 2xxx
    YY = (cclk_r_buf[8] - '0') * 10 + (cclk_r_buf[9] - '0') + 100;
    MM = (cclk_r_buf[11] - '0') * 10 + (cclk_r_buf[12] - '0');
    DD = (cclk_r_buf[14] - '0') * 10 + (cclk_r_buf[15] - '0');
    hh = (cclk_r_buf[17] - '0') * 10 + (cclk_r_buf[18] - '0');
    mm = (cclk_r_buf[20] - '0') * 10 + (cclk_r_buf[21] - '0');
    ss = (cclk_r_buf[23] - '0') * 10 + (cclk_r_buf[24] - '0');

    if (isLeap(YY + 1900)) {
        daysPerMonth[2] = 29;
    }

    // accumulate
    for (int i = 1; i <= 12; i++) {
        daysPerMonth[i] += daysPerMonth[i - 1];
    }

    epoch  = ss + mm * 60 + hh * 3600 + (daysPerMonth[ MM - 1] + DD - 1) * 86400 +
             (YY - 70) * 31536000L + ((YY - 69) / 4) * 86400L -
             ((YY - 1) / 100) * 86400L + ((YY + 299) / 400) * 86400L;

    snprintf(epoch_buf, sizeof(epoch_buf), "%.0f", epoch);
    printk("UTC epoch %s\n", epoch_buf);

    //if (stamp) {
     //   snprintf(stamp->data, sizeof(stamp->data), "%.0f", epoch);
     //   return stamp->data;
    //}

    return epoch;
}

float iotex_modem_get_battery_voltage(void)
{   
    enum at_cmd_state at_state;
    char vbat_ack[32], vbat[5];
    char *p=vbat_ack;

    int err = at_cmd_write("AT%XVBAT", vbat_ack, 32, &at_state); 

    if (err) {
        printk("Error when trying to do at_cmd_write: %d, at_state: %d", err, at_state);
    }

    p = strchr(p, ':') + 1;
    p++;   
    for(err=0; err < 4; err++)
    {
        vbat[err] = p[err];
    }
    vbat[4] = 0;
    return (atoi(vbat)/(float)1000.0);
}

void CheckPower(void)
{
    volatile float adc_voltage = 0;
    adc_voltage = iotex_modem_get_battery_voltage();
    if(adc_voltage < 3.3)
    {
        printk("power lower than 3.3 \n");
        PowerOffIndicator();
    }
}

