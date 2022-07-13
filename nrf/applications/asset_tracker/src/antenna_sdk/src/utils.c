#include <string.h>
#include <stdio.h>

/* Transform rau to iotx: 1 iotx = 10**18 rau */
const char *utils_rau2iotx(const char *rau, char *iotx, size_t max) {
    size_t r, w;
    size_t rau_len = strlen(rau);
    size_t decimal_point_pos, pad_zero;
    static const size_t transform_factor = 18;

    /* iotx buffer too short or empty */
    if (max < rau_len + 3 || max < transform_factor + 3 || !iotx) {
        return NULL;
    }

    if ('0' == rau[0]) {
        iotx[0] = '0';
        iotx[1] = 0;
        return iotx;
    }

    if (rau_len >= transform_factor) {
        decimal_point_pos = rau_len - transform_factor;

        for (r = 0, w = 0; r < rau_len;) {
            iotx[w++] = rau[r++];

            if (r == decimal_point_pos) {
                iotx[w++] = '.';
            }
        }
    }
    else {
        r = w = 0;
        iotx[w++] = '0';
        iotx[w++] = '.';
        pad_zero = transform_factor - rau_len;

        do {
            iotx[w++] = '0';
        } while (w < pad_zero + 2);

        do {
            iotx[w++] = rau[r++];
        } while (r < rau_len);

    }

    /* Ending */
    w -= 1;

    /* Remove useless zeros */
    while ('0' == iotx[w]) {
        --w;
    }

    /* Round number */
    if ('.' == iotx[w]) {
        --w;
    }

    iotx[w + 1] = 0;
    return iotx;
}
