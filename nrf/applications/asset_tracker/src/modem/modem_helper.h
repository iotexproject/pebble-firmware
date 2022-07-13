#ifndef _IOTEX_MODEM_H_
#define _IOTEX_MODEM_H_


#define MODEM_IMEI_LEN 15
#define TIMESTAMP_STR_LEN 50

typedef struct {
    char data[TIMESTAMP_STR_LEN];
} __attribute__((packed)) iotex_st_timestamp;

const char *iotex_modem_get_imei();
int iotex_model_get_signal_quality();
const char *iotex_modem_get_clock(iotex_st_timestamp *stamp);
double iotex_modem_get_clock_raw(iotex_st_timestamp *stamp);
float iotex_modem_get_battery_voltage(void);
void CheckPower(void);

#endif //_IOTEX_MODEM_H_