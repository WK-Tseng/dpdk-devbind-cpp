/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026, Tseng, Wei Kai
 */

/*
 * @file dpdk_var.hpp
 * @brief driver list and device details for DPDK device binding
 *
 * This info refers to the DPDK device binding information from DPDK 25.11 release, which can be found at:
 * https://github.com/DPDK/dpdk/blob/v25.11/usertools/dpdk-devbind.py
 */

#ifndef DPDK_VAR_HPP
#define DPDK_VAR_HPP

#include "dpdk_device_group.h"

#include <map>
#include <vector>
#include <set>
#include <string>

struct PciAdress {
    uint16_t domain;
    uint16_t bus;
    uint16_t dev;
    uint16_t func;
};

static
std::string
get_pci_address_string(
    uint16_t domain, uint16_t bus, 
    uint16_t dev, uint16_t func
) {
    char slot[32];
    snprintf(slot, sizeof(slot), "%04x:%02x:%02x.%d", domain, bus, dev, func);

    return slot;
}

static
std::string
get_pci_address_string(PciAdress& pci_address) {
    return get_pci_address_string(
        pci_address.domain,
        pci_address.bus,
        pci_address.dev,
        pci_address.func
    );
}

struct PciInfo {
    uint16_t id;
    std::string name;
};

struct DeviceInfo {
    PciAdress pci_address;
    PciInfo device_class;
    PciInfo vendor;
    PciInfo device;
    PciInfo sub_vendor;
    PciInfo sub_device;
    std::string s_driver;
    std::set<std::string> module_set;

    std::set<std::string> interfaces_set;
    bool b_ssh_if;
    bool b_active;

    int numa_node;
};

enum class NUMANodeStatus : int {
    INIT = -2,
    SINGLE = -1,
    ERROR = -3
};

// global dict ethernet devices present. Dictionary indexed by PCI address.
// Each device within this is itself a dictionary of device properties
std::map<std::string, DeviceInfo> devices;
std::vector<std::string> dpdk_drivers;
const std::vector<std::string> all_dpdk_drivers = {
    "igb_uio",
    "vfio-pci",
    "uio_pci_generic"
};

std::map<std::string, std::set<uint16_t>> network_class = {
    {"Class", {0x02}},
    {"Vendor", {}},
    {"Device", {}},
    {"SVendor", {}},
    {"SDevice", {}}
};

std::map<std::string, std::set<uint16_t>> acceleration_class = {
    {"Class", {0x12}},
    {"Vendor", {}},
    {"Device", {}},
    {"SVendor", {}},
    {"SDevice", {}}
};

std::map<std::string, std::set<uint16_t>> ifpga_class = {
    {"Class", {0x12}},
    {"Vendor", {0x8086}},
    {"Device", {0x0b30}},
    {"SVendor", {}},
    {"SDevice", {}}
};

std::map<std::string, std::set<uint16_t>> encryption_class = {
    {"Class", {0x10}},
    {"Vendor", {}},
    {"Device", {}},
    {"SVendor", {}},
    {"SDevice", {}}
};

std::map<std::string, std::set<uint16_t>> intel_processor_class = {
    {"Class", {0x0b}},
    {"Vendor", {0x8086}},
    {"Device", {}},
    {"SVendor", {}},
    {"SDevice", {}}
};

std::map<std::string, std::set<uint16_t>> cavium_sso = {
    {"Class", {0x08}},
    {"Vendor", {0x177d}},
    {"Device", {0xa04b, 0xa04d}},
    {"SVendor", {}},
    {"SDevice", {}}
};

std::map<std::string, std::set<uint16_t>> cavium_fpa = {
    {"Class", {0x08}},
    {"Vendor", {0x177d}},
    {"Device", {0xa053}},
    {"SVendor", {}},
    {"SDevice", {}}
};

std::map<std::string, std::set<uint16_t>> cavium_pkx = {
    {"Class", {0x08}},
    {"Vendor", {0x177d}},
    {"Device", {0xa0dd, 0xa049}},
    {"SVendor", {}},
    {"SDevice", {}}
};

std::map<std::string, std::set<uint16_t>> cavium_tim = {
    {"Class", {0x08}},
    {"Vendor", {0x177d}},
    {"Device", {0xa051}},
    {"SVendor", {}},
    {"SDevice", {}}
};

std::map<std::string, std::set<uint16_t>> cavium_zip = {
    {"Class", {0x12}},
    {"Vendor", {0x177d}},
    {"Device", {0xa037}},
    {"SVendor", {}},
    {"SDevice", {}}
};

std::map<std::string, std::set<uint16_t>> avp_vnic = {
    {"Class", {0x05}},
    {"Vendor", {0x1af4}},
    {"Device", {0x1110}},
    {"SVendor", {}},
    {"SDevice", {}}
};

std::map<std::string, std::set<uint16_t>> cnxk_bphy = {
    {"Class", {0x08}},
    {"Vendor", {0x177d}},
    {"Device", {0xa089}},
    {"SVendor", {}},
    {"SDevice", {}}
};

std::map<std::string, std::set<uint16_t>> cnxk_bphy_cgx = {
    {"Class", {0x08}},
    {"Vendor", {0x177d}},
    {"Device", {0xa059, 0xa060}},
    {"SVendor", {}},
    {"SDevice", {}}
};

std::map<std::string, std::set<uint16_t>> cnxk_dma = {
    {"Class", {0x08}},
    {"Vendor", {0x177d}},
    {"Device", {0xa081}},
    {"SVendor", {}},
    {"SDevice", {}}
};

std::map<std::string, std::set<uint16_t>> cnxk_inl_dev = {
    {"Class", {0x08}},
    {"Vendor", {0x177d}},
    {"Device", {0xa0f0, 0xa0f1}},
    {"SVendor", {}},
    {"SDevice", {}}
};

std::map<std::string, std::set<uint16_t>> hisilicon_dma = {
    {"Class", {0x08}},
    {"Vendor", {0x19e5}},
    {"Device", {0xa122}},
    {"SVendor", {}},
    {"SDevice", {}}
};

std::map<std::string, std::set<uint16_t>> odm_dma = {
    {"Class", {0x08}},
    {"Vendor", {0x177d}},
    {"Device", {0xa08c}},
    {"SVendor", {}},
    {"SDevice", {}}
};

std::map<std::string, std::set<uint16_t>> intel_dlb = {
    {"Class", {0x0b}},
    {"Vendor", {0x8086}},
    {"Device", {0x270b, 0x2710, 0x2714}},
    {"SVendor", {}},
    {"SDevice", {}}
};

std::map<std::string, std::set<uint16_t>> intel_ioat_bdw = {
    {"Class", {0x08}},
    {"Vendor", {0x8086}},
    {"Device", {0x6f20, 0x6f21, 0x6f22, 0x6f23, 0x6f24, 0x6f25, 0x6f26, 0x6f27, 0x6f2e, 0x6f2f}},
    {"SVendor", {}},
    {"SDevice", {}}
};

std::map<std::string, std::set<uint16_t>> intel_ioat_skx = {
    {"Class", {0x08}},
    {"Vendor", {0x8086}},
    {"Device", {0x2021}},
    {"SVendor", {}},
    {"SDevice", {}}
};

std::map<std::string, std::set<uint16_t>> intel_ioat_icx = {
    {"Class", {0x08}},
    {"Vendor", {0x8086}},
    {"Device", {0x0b00}},
    {"SVendor", {}},
    {"SDevice", {}}
};

std::map<std::string, std::set<uint16_t>> intel_idxd_spr = {
    {"Class", {0x08}},
    {"Vendor", {0x8086}},
    {"Device", {0x0b25}},
    {"SVendor", {}},
    {"SDevice", {}}
};

std::map<std::string, std::set<uint16_t>> intel_idxd_gnrd = {
    {"Class", {0x08}},
    {"Vendor", {0x8086}},
    {"Device", {0x11fb}},
    {"SVendor", {}},
    {"SDevice", {}}
};

std::map<std::string, std::set<uint16_t>> intel_idxd_dmr = {
    {"Class", {0x08}},
    {"Vendor", {0x8086}},
    {"Device", {0x1212}},
    {"SVendor", {}},
    {"SDevice", {}}
};

std::map<std::string, std::set<uint16_t>> intel_ntb_skx = {
    {"Class", {0x06}},
    {"Vendor", {0x8086}},
    {"Device", {0x201c}},
    {"SVendor", {}},
    {"SDevice", {}}
};

std::map<std::string, std::set<uint16_t>> intel_ntb_icx = {
    {"Class", {0x06}},
    {"Vendor", {0x8086}},
    {"Device", {0x347e}},
    {"SVendor", {}},
    {"SDevice", {}}
};

std::map<std::string, std::set<uint16_t>> cnxk_sso = {
    {"Class", {0x08}},
    {"Vendor", {0x177d}},
    {"Device", {0xa0f9, 0xa0fa}},
    {"SVendor", {}},
    {"SDevice", {}}
};

std::map<std::string, std::set<uint16_t>> cnxk_npa = {
    {"Class", {0x08}},
    {"Vendor", {0x177d}},
    {"Device", {0xa0fb, 0xa0fc}},
    {"SVendor", {}},
    {"SDevice", {}}
};

std::map<std::string, std::set<uint16_t>> cn9k_ree = {
    {"Class", {0x08}},
    {"Vendor", {0x177d}},
    {"Device", {0xa0f4}},
    {"SVendor", {}},
    {"SDevice", {}}
};

std::map<std::string, std::set<uint16_t>> virtio_blk = {
    {"Class", {0x01}},
    {"Vendor", {0x1af4}},
    {"Device", {0x1001,1042}},
    {"SVendor", {}},
    {"SDevice", {}}
};

std::map<std::string, std::set<uint16_t>> cnxk_ml = {
    {"Class", {0x08}},
    {"Vendor", {0x177d}},
    {"Device", {0xa092}},
    {"SVendor", {}},
    {"SDevice", {}}
};

std::vector<std::map<std::string, std::set<uint16_t>>> network_devices = {
    network_class,
    cavium_pkx,
    avp_vnic,
    ifpga_class
};

std::vector<std::map<std::string, std::set<uint16_t>>> baseband_devices  = {
    acceleration_class
};

std::vector<std::map<std::string, std::set<uint16_t>>> crypto_devices = {
    encryption_class,
    intel_processor_class
};

std::vector<std::map<std::string, std::set<uint16_t>>> dma_devices = {
    cnxk_dma,
    hisilicon_dma,
    intel_idxd_gnrd,
    intel_idxd_dmr,
    intel_idxd_spr,
    intel_ioat_bdw,
    intel_ioat_icx,
    intel_ioat_skx,
    odm_dma
};

std::vector<std::map<std::string, std::set<uint16_t>>> eventdev_devices = {
    cavium_sso,
    cavium_tim,
    intel_dlb,
    cnxk_sso
};

std::vector<std::map<std::string, std::set<uint16_t>>> mempool_devices = {
    cavium_fpa,
    cnxk_npa
};

std::vector<std::map<std::string, std::set<uint16_t>>> compress_devices = {
    cavium_zip
};

std::vector<std::map<std::string, std::set<uint16_t>>> regex_devices = {
    cn9k_ree
};

std::vector<std::map<std::string, std::set<uint16_t>>> ml_devices = {
    cnxk_ml
};

std::vector<std::map<std::string, std::set<uint16_t>>> misc_devices = {
    cnxk_bphy,
    cnxk_bphy_cgx,
    cnxk_inl_dev,
    intel_ntb_skx,
    intel_ntb_icx,
    virtio_blk
};

std::map<int, std::vector<std::map<std::string, std::set<uint16_t>>>> device_group_map = {
    {DeviceGroup::NETWORK, network_devices},
    {DeviceGroup::BASEBAND, baseband_devices},
    {DeviceGroup::CRYPTO, crypto_devices},
    {DeviceGroup::DMA, dma_devices},
    {DeviceGroup::EVENTDEV, eventdev_devices},
    {DeviceGroup::MEMPOOL, mempool_devices},
    {DeviceGroup::COMPRESS, compress_devices},
    {DeviceGroup::REGEX, regex_devices},
    {DeviceGroup::ML, ml_devices},
    {DeviceGroup::MISC, misc_devices}
};

#endif // DPDK_VAR_HPP