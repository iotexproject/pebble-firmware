#
# Copyright (c) 2019 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
#

zephyr_include_directories(.)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/config.c)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/endian_conv.c)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/iotex_emb.c)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/parse.c)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pb_pack.c)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pb_proto.c)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/request.c)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/response.c)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/rule.c)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/signer.c)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/u128.c)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/utils.c)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/abi_pack.c)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/abi_read_contract.c)

