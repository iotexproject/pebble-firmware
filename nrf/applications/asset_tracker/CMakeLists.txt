#
# Copyright (c) 2018 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
#

cmake_minimum_required(VERSION 3.8.2)

set(spm_CONF_FILE ${CMAKE_CURRENT_SOURCE_DIR}/spm.conf)


set(PM_STATIC_YML_FILE
  ${CMAKE_CURRENT_SOURCE_DIR}/configuration/${BOARD}/pm_static.yml
  )

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(asset_tracker)
zephyr_compile_definitions(PROJECT_NAME=${PROJECT_NAME})
zephyr_compile_definitions(_POSIX_C_SOURCE=200809L)

# NORDIC SDK APP START
target_sources(app PRIVATE src/main.c)
# BME680
target_sources(app PRIVATE src/bme/bme680.c)
target_sources(app PRIVATE src/bme/bme680_helper.c)

# HAL
target_sources(app PRIVATE src/hal/hal_adc.c)
target_sources(app PRIVATE src/hal/hal_gpio.c)

# Modem
target_sources(app PRIVATE src/modem/modem_helper.c)
# MQTT
target_sources(app PRIVATE src/mqtt/mqtt.c)
target_sources(app PRIVATE src/mqtt/config.c)
target_sources(app PRIVATE src/mqtt/payload.c)
# Protbuf
#target_sources(app PRIVATE src/nanopb/pb_encode.c)
#target_sources(app PRIVATE src/nanopb/pb_decode.c)
#target_sources(app PRIVATE src/nanopb/pb_common.c)
# Local NVS
target_sources(app PRIVATE src/nvs/local_storage.c)
# Signature 
target_sources(app PRIVATE src/sign/ecdsa.c)
# OTA
target_sources(app PRIVATE src/ota/http_update.c)
# Unittest
#target_sources(app PRIVATE src/unittest/unittest.c)
#target_sources(app PRIVATE src/unittest/nvs_unittest.c)
#target_sources(app PRIVATE src/unittest/mqtt_unittest.c)
#target_sources(app PRIVATE src/unittest/nanopb_unittest.c)
#target_sources(app PRIVATE src/unittest/nanopb_unittest.pb.c)

target_sources(app PRIVATE src/icm/icm42605_helper.c)
target_sources(app PRIVATE src/icm/Icm426xxDriver_HL_apex.c)
target_sources(app PRIVATE src/icm/Icm426xxDriver_HL.c)
target_sources(app PRIVATE src/icm/Icm426xxTransport.c)

# Tide:  ICM Action detect Debug codec
target_sources(app PRIVATE src/icm/tiltConfig.c)
target_sources(app PRIVATE src/icm/womConfig.c)
target_sources(app PRIVATE src/icm/smdConfig.c)
target_sources(app PRIVATE src/icm/icm426xxActionDetect.c)

target_sources(app PRIVATE src/light_sensor/tsl2572.c)
# NORDIC SDK APP END
zephyr_include_directories(src)

# Include application events and configuration headers
zephyr_library_include_directories(
  src/orientation_detector
  src/ui
  src/cloud_codec
#  src/unittest
#  src/nanopb
  src/mqtt
  src/env_sensors
  src/light_sensor
  src/watchdog
  src/sign
  src/antenna_sdk/src
  src/antenna_sdk/src/crypto
  )

# Application sources
add_subdirectory(src/motion)
add_subdirectory(src/orientation_detector)
add_subdirectory(src/ui)
add_subdirectory(src/cloud_codec)
add_subdirectory(src/gps_controller)
add_subdirectory(src/env_sensors)
add_subdirectory_ifdef(CONFIG_WATCHDOG src/watchdog)
#add_subdirectory_ifdef(CONFIG_LIGHT_SENSOR src/light_sensor)
add_subdirectory(src/antenna_sdk/src)
add_subdirectory(src/antenna_sdk/src/crypto)

if (CONFIG_USE_BME680_BSEC)
  target_link_libraries(app PUBLIC bsec_lib)
endif()
