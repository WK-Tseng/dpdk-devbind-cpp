/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026, Tseng, Wei Kai
 */


#include "dpdk_devbind.h"
#include "dpdk_device_group.h"
#include "dpdk_scan_driver.hpp"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @brief display the status of devices and their drivers.
 */
void
show_status() {
    check_modules();

    get_device_details(DeviceGroup::NETWORK);
    get_device_details(DeviceGroup::BASEBAND);
    get_device_details(DeviceGroup::CRYPTO);
    get_device_details(DeviceGroup::DMA);
    get_device_details(DeviceGroup::EVENTDEV);
    get_device_details(DeviceGroup::MEMPOOL);
    get_device_details(DeviceGroup::COMPRESS);
    get_device_details(DeviceGroup::REGEX);
    get_device_details(DeviceGroup::ML);
    get_device_details(DeviceGroup::MISC);

    show_device_status(DeviceGroup::NETWORK, "Network", true);
    show_device_status(DeviceGroup::BASEBAND, "Baseband");
    show_device_status(DeviceGroup::CRYPTO, "Crypto");
    show_device_status(DeviceGroup::DMA, "DMA");
    show_device_status(DeviceGroup::EVENTDEV, "Eventdev");
    show_device_status(DeviceGroup::MEMPOOL, "Mempool");
    show_device_status(DeviceGroup::COMPRESS, "Compress");
    show_device_status(DeviceGroup::MISC, "Misc (rawdev)");
    show_device_status(DeviceGroup::REGEX, "Regex");
    show_device_status(DeviceGroup::ML, "ML");
}

#ifdef __cplusplus
}
#endif
