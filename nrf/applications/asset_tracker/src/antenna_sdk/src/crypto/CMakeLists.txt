#
# Copyright (c) 2019 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
#

zephyr_include_directories(.)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/bignum.c)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ecdsa.c)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/hmac_drbg.c)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/memzero.c)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/rand.c)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/rfc6979.c)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/secp256k1.c)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/sha2.c)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/sha3.c)
