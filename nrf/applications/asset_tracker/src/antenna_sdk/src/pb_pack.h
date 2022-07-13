#ifndef _IOTEX_PB_PACK_H_
#define _IOTEX_PB_PACK_H_

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

typedef enum {
    PB_WT_VARINT = 0,
    PB_WT_64 = 1,
    PB_WT_LD = 2,
    PB_WT_32 = 5,
    PB_WT_EMB = 0x82
} pb_em_wtype;

typedef enum {
    PB_ERR_OK = 0,
    PB_ERR_WTYPE = 1,
    PB_ERR_FIELD = 2,
    PB_ERR_HEADER = 3,
    PB_ERR_ENCBUF = 4,
    PB_ERR_DECBUF = 5
} pb_em_error;

typedef union {
    uint8_t bytes[2];
    uint16_t u_data;
} pb_un_u16conv;

typedef union {
    uint8_t bytes[4];
    uint32_t u_data;
} pb_un_u32conv;

typedef union {
    uint8_t bytes[8];
    uint64_t u_data;
} pb_un_u64conv;

/* NULL means it's an optional field */
typedef struct {
    /* write type one of pb_em_wtype or PB_EMBMSG_MASK | PB_WT_LD */
    uint8_t type;

    /* field number */
    uint8_t field;

    /* data to write, for embedded messages, it pointer to embedded message items list */
    void *data;

    /* for length-delimited type use, indicate data length in byte, for embedded messages it means embedded message item length */
    uint8_t length;
} pb_st_item;

#define PB_MAX_TYPE (PB_WT_32)
#define PB_ONE_BYTE_SIZE    1

#define PB_VARINT_MASK 0x7fL
#define PB_VARINT_MORE 0x80

/* Field numbers 19000 through 19999 are reserved for the protocol buffer library implementation. */
#define PB_FIELD_RESV_MIN 19000
#define PB_FIELD_RESV_MAX 19999
#define PB_FILED_MAX  0x1fffffff

#define PB_WTYPE_MASK	0x7L
#define PB_EMBMSG_MASK 0x80

#define PB_GET_WTYPE(x) ((x) & PB_WTYPE_MASK)
#define PB_GET_FIELD(x) (((x) >> 3) & PB_FILED_MAX)

#define IS_EMB_MSG(x) (PB_GET_WTYPE(x) == PB_WT_LD && ((x) & (PB_EMBMSG_MASK)))
#define PB_SET_HEADER(field, type) ((((field) & PB_FILED_MAX) << 3) | ((type) & PB_WTYPE_MASK))

#define PB_32_ITEM(field, data)  	    {PB_WT_32, (field), &data, sizeof(data)}
#define PB_64_ITEM(field, data)  	    {PB_WT_64, (field), &data, sizeof(data)}
#define PB_VARINT_ITEM(field, data)    	{PB_WT_VARINT, (field), &data, sizeof(data)}
#define PB_STRING_ITEM(field, data)     {PB_WT_LD, (field), data, strlen(data)}
#define PB_EMBMSG_ITEM(field, data)   	{PB_WT_EMB, (field), &data, sizeof(data) / sizeof(data[0])}

typedef int(*PB_ENCODE_FUNC)(const pb_st_item *item, uint8_t *buffer, int32_t size);

int pb_encode_varint(uint64_t varint, uint8_t *buffer, int32_t size);
int pb_pack(const pb_st_item *messages, size_t len, uint8_t *buffer, size_t size);

#ifdef	__cplusplus
}
#endif


#endif /* _IOTEX_PB_PACK_H_ */
