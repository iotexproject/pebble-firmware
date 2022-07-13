#include "unittest.h"
#include "nvs/local_storage.h"
#include <assert.h>

#define TEST_COUNT 10

static void test_write() {

    int i;
    iotex_storage_devinfo dev;

    /* Exist delete */
    if (iotex_local_storage_load(SID_DEVICE_INFO, &dev, sizeof(dev)) == 0) {
        UNITTEST_ASSERT_EQ(iotex_local_storage_del(SID_DEVICE_INFO), 0);
    }

    /* Write test data */
    for (i = 0; i < TEST_COUNT; i++) {
        dev.snr = i * 10;
        dev.vbatx100 = 4 * 100 + dev.snr;
        UNITTEST_ASSERT_EQ(iotex_local_storage_save(SID_DEVICE_INFO, &dev, sizeof(dev)), 0);
    }

    UNITTEST_AUTO_PASS();
}

static void test_read() {

    int i, count;
    iotex_storage_devinfo infos[TEST_COUNT];
    size_t item_len = sizeof(iotex_storage_devinfo);

    UNITTEST_ASSERT_EQ(iotex_local_storage_load(SID_DEVICE_INFO, infos, sizeof(infos)), -1);

    /* Test read part of data */
    count = iotex_local_storage_readall(SID_DEVICE_INFO, infos, item_len * 4, item_len);
    UNITTEST_ASSERT_EQ(count, 4);

    for (i = count - 1; i >= 0; i--) {
        UNITTEST_ASSERT_EQ(infos[i].snr,  (TEST_COUNT - i - 1) * 10);
        UNITTEST_ASSERT_EQ(infos[i].vbatx100, 4 * 100 + infos[i].snr);
    }

    /* Test read all data*/
    count = iotex_local_storage_readall(SID_DEVICE_INFO, infos, sizeof(infos), item_len);
    UNITTEST_ASSERT_EQ(count, TEST_COUNT);

    for (i = count - 1; i >= 0; i--) {
        UNITTEST_ASSERT_EQ(infos[i].snr,  (TEST_COUNT - i - 1) * 10);
        UNITTEST_ASSERT_EQ(infos[i].vbatx100, 4 * 100 + infos[i].snr);
    }

    /* Test read buffer too short */
    UNITTEST_ASSERT_EQ(iotex_local_storage_readall(SID_DEVICE_INFO, infos, item_len - 1, item_len), 0);
    UNITTEST_AUTO_PASS();
}

static void test_read_history() {

    iotex_storage_devinfo first, last;

    /* LIFO Zero always indicate the latest one */
    UNITTEST_ASSERT_EQ(iotex_local_storage_hist(SID_DEVICE_INFO, &last, sizeof(last), 0), 0);
    UNITTEST_ASSERT_EQ(iotex_local_storage_hist(SID_DEVICE_INFO, &first, sizeof(first), TEST_COUNT), -1);
    UNITTEST_ASSERT_EQ(iotex_local_storage_hist(SID_DEVICE_INFO, &first, sizeof(first), TEST_COUNT - 1), 0);

    UNITTEST_ASSERT_EQ(first.snr, 0);
    UNITTEST_ASSERT_EQ(first.vbatx100, 400);
    UNITTEST_ASSERT_EQ(last.snr, 90);
    UNITTEST_ASSERT_EQ(last.vbatx100, 490);

    UNITTEST_AUTO_PASS();
}


int nvs_unittest(void) {

    /* Must first */
    test_write();

    test_read();
    test_read_history();

    /* Finally clear all test data */
    UNITTEST_AUTO_TRUE(iotex_local_storage_del(SID_DEVICE_INFO) == 0);
    return 0;
}
