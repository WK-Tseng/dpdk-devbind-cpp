/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026, Tseng, Wei Kai
 */

/*
 * @file dpdk_device_group.h
 * @brief library header for DPDK device groups
 */

#ifndef DPDK_DEVICE_GROUP_H
#define DPDK_DEVICE_GROUP_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    NETWORK = 0,
    BASEBAND = 1,
    CRYPTO = 2,
    DMA = 3,
    EVENTDEV = 4,
    MEMPOOL = 5,
    COMPRESS = 6,
    REGEX = 7,
    ML = 8,
    MISC = 9
} DeviceGroup;

#ifdef __cplusplus
}
#endif

#endif // DPDK_DEVICE_GROUP_H