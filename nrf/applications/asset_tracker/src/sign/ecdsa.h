#ifndef	__ESDA_H__
#define	__ESDA_H__

int initECDSA_sep256r(void);
int doESDA_sep256r_Sign(char *inbuf, uint32_t len, char *buf, int* sinlen);
void hex2str(char* buf_hex, int len, char *str);
int GenRandom(char *out);
int startup_check_ecc_key(void);
int  get_ecc_public_key(char *pub);

#endif