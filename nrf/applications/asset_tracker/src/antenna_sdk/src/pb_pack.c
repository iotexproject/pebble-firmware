#include <string.h>
#include <assert.h>
#include "pb_pack.h"
#include "endian_conv.h"

/* Check if varint value is zero, is zero return true, else false */
static inline int pb_varint_is_zero(const pb_st_item *item) {
    int _true = 1;
    int _false = 0;

    if (item->type != PB_WT_VARINT) {
        return _false;
    }

    uint32_t i;
    const uint8_t *data = item->data;
    uint32_t len = item->length ? item->length : sizeof(uint64_t);

    for (i = 0; i < len; i++) {
        if (data[i]) {
            return _false;
        }
    }

    return _true;
}

static int pb_pack_varint(const pb_st_item *item, uint8_t *buffer, int32_t size) {
    int32_t i = 0;
    uint64_t varint;
    uint8_t *value = buffer;
    const uint8_t *bytes = item->data;
    pb_un_u16conv u16 = {{bytes[0], bytes[1]}};
    pb_un_u32conv u32 = {{bytes[0], bytes[1], bytes[2], bytes[3]}};
    pb_un_u64conv u64 = {{bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5], bytes[6], bytes[7]}};

    switch (item->length) {
        case sizeof(uint8_t):
            varint = bytes[0];
            break;

        case sizeof(uint16_t):
            varint = ENDIAN_H2LE16(u16.u_data);
            break;

        case sizeof(uint32_t):
            varint = ENDIAN_H2LE32(u32.u_data);
            break;

        default:
        case sizeof(uint64_t):
            varint = ENDIAN_H2LE64(u64.u_data);
            break;
    }

    for (i = 0; i < size && varint; i++) {
        *value = PB_VARINT_MORE | (varint & PB_VARINT_MASK);
        varint >>= 7;
        value++;
    }

    /* Varint value is zero or buffer too short */
    if (value == buffer) {
        return 0;
    }

    *(--value) &= ~PB_VARINT_MORE;
    return i;
}

static int pb_pack_64bit(const pb_st_item *item, uint8_t *buffer, int32_t size) {
    if (size < sizeof(uint64_t)) {
        return -PB_ERR_ENCBUF;
    }

    const uint8_t *bytes = item->data;
    pb_un_u64conv conv = {{bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5], bytes[6], bytes[7]}};

    conv.u_data = ENDIAN_H2LE64(conv.u_data);
    memcpy(buffer, conv.bytes, sizeof(uint64_t));
    return sizeof(uint64_t);
}

static int pb_pack_32bit(const pb_st_item *item, uint8_t *buffer, int32_t size) {
    if (size < sizeof(uint32_t)) {
        return -PB_ERR_ENCBUF;
    }

    const uint8_t *bytes = item->data;
    pb_un_u32conv conv = {{bytes[0], bytes[1], bytes[2], bytes[3]}};

    conv.u_data = ENDIAN_H2LE32(conv.u_data);
    memcpy(buffer, conv.bytes, sizeof(uint32_t));
    return sizeof(uint32_t);
}

int pb_encode_varint(uint64_t varint, uint8_t *buffer, int32_t size) {
    if (varint <= PB_VARINT_MASK) {
        *buffer = varint & PB_VARINT_MASK;
        return 1;
    }

    pb_st_item item = PB_VARINT_ITEM(0, varint);
    return pb_pack_varint(&item, buffer, size);
}

int pb_encode_header(uint8_t *buffer, uint32_t size, uint8_t type, uint32_t field) {
    if (!field || field > PB_FILED_MAX || (field >= PB_FIELD_RESV_MIN && field <= PB_FIELD_RESV_MAX)) {
        return -PB_ERR_FIELD;
    }

    field = PB_SET_HEADER(field, type);

    /* Filed and wtype occupied one byte (frequently filed number)*/
    if (field < PB_VARINT_MASK) {
        *buffer = field & PB_VARINT_MASK;
        return 1;
    }

    pb_st_item header = PB_VARINT_ITEM(0, field);
    return pb_pack_varint(&header, buffer, size);
}

static int pb_pack_ld(const pb_st_item *item, uint8_t *buffer, int32_t size) {
    int varint_length;

    /* Pack length first */
    if ((varint_length = pb_encode_varint(item->length, buffer, size)) < 0) {
        return -PB_ERR_ENCBUF;
    }

    /* Copy value */
    memcpy(buffer + varint_length, item->data, item->length);
    return varint_length + item->length;
}

static const PB_ENCODE_FUNC __encoder[PB_MAX_TYPE + 1] = {
    pb_pack_varint,
    pb_pack_64bit,
    pb_pack_ld,
    NULL,
    NULL,
    pb_pack_32bit,
};

int pb_pack(const pb_st_item *message, size_t len, uint8_t *buffer, size_t size) {
    int i = 0;
    int packed_len = 0;
    int header_len = 0, value_len = 0;

    int emb_msg_len;
    uint8_t write_type = 0;
    uint8_t emb_msg_len_buffer[8];
    int embmsg_len_occupy_bytes = 0;

    PB_ENCODE_FUNC encode = NULL;

    for (i = 0; i < len; i++, message++) {
        write_type = PB_GET_WTYPE(message->type);

        if (write_type > PB_MAX_TYPE) {
            return -PB_ERR_WTYPE;
        }

        /* Skip deprecated type and optional field */
        if (!(encode = __encoder[write_type]) || !message->data) {
            continue;
        }

#ifdef _DEBUG_PB_ENCODE_
        fprintf(stdout, "P[%03d/%03u], Msg[%02x], T[%02u], F[%u]\n", packed_len, size, i, write_type, message->field);
#endif

        /* Pack embedded message */
        if (IS_EMB_MSG(message->type)) {
            /* Embedded message key and type£¬header length must geater than zero  */
            if ((header_len = pb_encode_header(buffer + packed_len, size - packed_len, write_type, message->field)) <= 0) {
                return -PB_ERR_ENCBUF;
            }

            packed_len += header_len;

            /* Currently do not know embedded msg length, default preserver 1 byte for length (<= 127) */
            emb_msg_len = pb_pack((const pb_st_item *)message->data,
                                  message->length,
                                  buffer + packed_len + PB_ONE_BYTE_SIZE,
                                  size - packed_len - PB_ONE_BYTE_SIZE);

#ifdef _DEBUG_PB_ENCODE_
            fprintf(stdout, "Embedded msg length: %d\n", emb_msg_len);
#endif

            if (emb_msg_len < 0) {
                return emb_msg_len;
            }

            /* Length itself(an varint) occupy more then 1 byte */
            if (emb_msg_len > PB_VARINT_MASK) {
                /* Encode emb msg length(varint) and get itself occupied bytes */
                embmsg_len_occupy_bytes = pb_encode_varint(emb_msg_len, emb_msg_len_buffer, sizeof(emb_msg_len_buffer));

                /* Move ld data to back */
                memmove(buffer + packed_len + embmsg_len_occupy_bytes, buffer + packed_len + PB_ONE_BYTE_SIZE, emb_msg_len);

                /* Copy embbed message length */
                memcpy(buffer + packed_len, emb_msg_len_buffer, embmsg_len_occupy_bytes);
                packed_len += embmsg_len_occupy_bytes + emb_msg_len;
            }
            /* Length occupied 1 byte */
            else {
                buffer[packed_len] = emb_msg_len;

                /* Add embmsg length and length occupied size */
                packed_len += emb_msg_len + PB_ONE_BYTE_SIZE;
            }
        }
        else {
            /* Varint do not encode zero value */
            if (PB_WT_VARINT == write_type && pb_varint_is_zero(message)) {
                continue;
            }

            /* Key (field and write type) */
            if ((header_len = pb_encode_header(buffer + packed_len, size - packed_len, message->type, message->field)) < 0) {
                return -PB_ERR_ENCBUF;
            }

            /* Add key length */
            packed_len += header_len;

            /* Value */
            if ((value_len = encode(message, buffer + packed_len, size - packed_len)) < 0) {
                return value_len;
            }

            /* Add value length */
            packed_len += value_len;
        }

        /* Buffer to short */
        if (packed_len >= size) {
            return -PB_ERR_ENCBUF;
        }
    }

    return packed_len;
}
