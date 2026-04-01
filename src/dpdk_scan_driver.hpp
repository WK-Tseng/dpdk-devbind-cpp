/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026, Tseng, Wei Kai
 */

/*
 * @file dpdk_scan_driver.hpp
 * @brief driver list and device details for DPDK device binding
 *
 * This info refers to the DPDK device binding information from DPDK 25.11 release, which can be found at:
 * https://github.com/DPDK/dpdk/blob/v25.11/usertools/dpdk-devbind.py
 */

#ifndef DPDK_SCAN_DRIVER_HPP
#define DPDK_SCAN_DRIVER_HPP

#include <map>
#include <vector>
#include <set>
#include <string>

void get_device_details(int device_group);
void show_device_status(int device_group, std::string device_name, bool if_field = false);
void check_modules();

#endif // DPDK_SCAN_DRIVER_HPP
