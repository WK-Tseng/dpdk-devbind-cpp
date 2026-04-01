/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026, Tseng, Wei Kai
 */

/*
 * @file dpdk_devbind.h
 * @brief library header for DPDK device binding
 */

#ifndef DPDK_DEVBIND_H
#define DPDK_DEVBIND_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pci/pci.h>
#include <libkmod.h>

void show_status();

#ifdef __cplusplus
}
#endif

#endif // DPDK_DEVBIND_H
