/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026, Tseng, Wei Kai
 */

#include "dpdk_var.hpp"
#include "dpdk_scan_driver.hpp"

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <sstream>

extern "C" {
    #include <sys/utsname.h>
    #include <pci/pci.h>
    #include <libkmod.h>
}

namespace fs = std::filesystem;

#define DRIVER_BUF_SIZE 1024

void update_pci_device_netwrok_interface_name(DeviceInfo& dev_info);
void update_pci_device_numa_node(DeviceInfo& dev_info);
bool module_is_loaded(const std::string& module);

static std::set<std::string>
get_ssh_interface_set() {
    std::set<std::string> ssh_interface_set;

    fs::path path = "/proc/net/route";
    if (fs::exists(path)) {
        std::ifstream route_file(path);
        if (route_file.is_open()) {
            std::string line_str;
            while (std::getline(route_file, line_str)) {
                std::stringstream ss(line_str);
                std::string word;
                std::string pre_word;
                int count = 0;
                while (ss >> word) {
                    count++;
                    if (count == 2 && word.length() == 8 && word.substr(4, 8) != "FEA9") {
                        ssh_interface_set.insert(pre_word);
                    } else if (count > 2) {
                        break;
                    }
                    pre_word = word;
                }
            }
        }
    }

    return ssh_interface_set;
}

static bool
device_type_match(
    DeviceInfo& dev,
    std::vector<std::map<std::string, std::set<uint16_t>>>& devices_type
) {
    for (const auto& device_type : devices_type) {
        int param_count = 0;
        int match_count = 0;

        for (const auto& device_pair : device_type) {
            if (!device_pair.second.empty()) {
                param_count++;
            }
        }

        if (device_type.at("Class").count(dev.device_class.id >> 8) > 0) {
            match_count++;
            for (const auto& device_pair : device_type) {
                if (!device_pair.second.empty()) {
                    if (device_pair.first == "Class") {
                        continue; // already matched
                    } else if (device_pair.first == "Vendor" && device_pair.second.count(dev.vendor.id) > 0) {
                        match_count++;
                    } else if (device_pair.first == "Device" && device_pair.second.count(dev.device.id) > 0) {
                        match_count++;
                    } else if (device_pair.first == "SVendor" && device_pair.second.count(dev.sub_vendor.id) > 0) {
                        match_count++;
                    } else if (device_pair.first == "SDevice" && device_pair.second.count(dev.sub_device.id) > 0) {
                        match_count++;
                    }
                }
            }

            if (match_count == param_count) {
                return true;
            }
        }
    }
    return false;
}

#if PCI_LIB_VERSION < 0x030800

static struct kmod_ctx *kmod_ctx;

static std::string
find_driver(pci_dev& dev) {
    char *base;

    if (dev.access->method != PCI_ACCESS_SYS_BUS_PCI) {
        return "";
    }

    base = pci_get_param(dev.access, const_cast<char*>("sysfs.path"));
    if (!base || !base[0]) {
        return "";
    }

    try {
        auto slot = get_pci_address_string(dev.domain, dev.bus, dev.dev, dev.func);

        fs::path driver_path = fs::path(base) / "devices" / slot / "driver";
        if (fs::exists(driver_path) && fs::is_symlink(driver_path)) {
            fs::path target = fs::read_symlink(driver_path);
            return target.filename().string();
        }
    } catch (const std::exception& e) {
        // Handle filesystem errors if necessary
        return "";
    }
    
    return "";
}

static uint16_t
find_sub_vendor_id(pci_dev& dev) {
    char *base;

    if (dev.access->method != PCI_ACCESS_SYS_BUS_PCI) {
        return 0;
    }

    base = pci_get_param(dev.access, const_cast<char*>("sysfs.path"));
    if (!base || !base[0]) {
        return 0;
    }

    try {
        auto slot = get_pci_address_string(dev.domain, dev.bus, dev.dev, dev.func);

        fs::path driver_path = fs::path(base) / "devices" / slot / "subsystem_vendor";
        if (fs::exists(driver_path)) {
            std::ifstream vendor_file(driver_path);
            if (vendor_file.is_open()) {
                std::string vendor_id_str;
                std::getline(vendor_file, vendor_id_str);
                return static_cast<uint16_t>(std::stoul(vendor_id_str, nullptr, 16));
            }
        }
    } catch (const std::exception& e) {
        // Handle filesystem errors if necessary
        return 0;
    }

    return 0;
}

static uint16_t
find_sub_device_id(pci_dev& dev) {
    char *base;

    if (dev.access->method != PCI_ACCESS_SYS_BUS_PCI) {
        return 0;
    }

    base = pci_get_param(dev.access, const_cast<char*>("sysfs.path"));
    if (!base || !base[0]) {
        return 0;
    }

    try {
        auto slot = get_pci_address_string(dev.domain, dev.bus, dev.dev, dev.func);

        fs::path driver_path = fs::path(base) / "devices" / slot / "subsystem_device";
        if (fs::exists(driver_path)) {
            std::ifstream vendor_file(driver_path);
            if (vendor_file.is_open()) {
                std::string vendor_id_str;
                std::getline(vendor_file, vendor_id_str);
                return static_cast<uint16_t>(std::stoul(vendor_id_str, nullptr, 16));
            }
        }
    } catch (const std::exception& e) {
        // Handle filesystem errors if necessary
        return 0;
    }

    return 0;
}

static std::string
next_module(pci_dev& dev) {
    static kmod_list *klist, *kcurrent;
    static kmod_module *kmodule;

    if (kmodule) {
        kmod_module_unref(kmodule);
        kmodule = NULL;
    }

    if (!klist) {
        pci_fill_info(&dev, PCI_FILL_MODULE_ALIAS);
        if (!dev.module_alias) {
            return "";
        }
	
        if (auto err = kmod_module_new_from_lookup(kmod_ctx, dev.module_alias, &klist); err < 0)
        {
            std::cerr << "libkmod lookup failed: error " << err << std::endl;
            return "";
        }
        kcurrent = klist;
    } else {
        kcurrent = kmod_list_next(klist, kcurrent);
    }
    

    if (kcurrent) {
      kmodule = kmod_module_get_module(kcurrent);
      return kmod_module_get_name(kmodule);
    }

    kmod_module_unref_list(klist);
    klist = NULL;
    return "";
}

static std::set<std::string>
get_modules(pci_dev& dev) {
    if (!kmod_ctx) {
        kmod_ctx = kmod_new(NULL, NULL);
        if (!kmod_ctx) {
            std::cerr << "libkmod context creation failed" << std::endl;
            return {};
        }

        if (auto err = kmod_load_resources(kmod_ctx); err < 0) {
            std::cerr << "Unable to load libkmod resources: error " << err << std::endl;
            return {};
        }
    }

    std::set<std::string> modules;
    std::string module;

    while (!(module = next_module(dev)).empty()) {
        if (modules.count(module) == 0) {
            modules.insert(module);
        }
    }
    
    return modules;
}

/*
 * @brief get device details for a specific device type
 *
 * This function populates the "devices" dictionary. The keys used are
 * the pci addresses (domain:bus:dev.func). The values are themselves
 * dictionaries - one for each NIC.
 * 
 * @param devices_type: a vector of maps, where each map contains the details of a device type, including Class, Vendor, Device, SVendor, and SDevice
 */
void
get_device_details(int device_group) {
    std::vector<std::map<std::string, std::set<uint16_t>>> devices_type;
    devices_type = device_group_map[device_group];


    pci_access *pacc;
    pci_dev *dev;
    char namebuf[DRIVER_BUF_SIZE];
    char driverbuf[DRIVER_BUF_SIZE];

    pacc = pci_alloc();
    pci_init(pacc);
    pci_scan_bus(pacc);

    for (dev = pacc->devices; dev; dev = dev->next) {
        DeviceInfo dev_info;

#if PCI_LIB_VERSION < 0x030800
        // not support PCI_FILL_DRIVER
        pci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_BASES | PCI_FILL_CLASS | PCI_FILL_MODULE_ALIAS);
#else
        pci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_BASES | PCI_FILL_CLASS | PCI_FILL_DRIVER | PCI_FILL_MODULE_ALIAS);
#endif

        // format the PCI address as a string
        dev_info.pci_address.domain = dev->domain;
        dev_info.pci_address.bus = dev->bus;
        dev_info.pci_address.dev = dev->dev;
        dev_info.pci_address.func = dev->func;

        // get device class
        dev_info.device_class.id = dev->device_class;
        dev_info.device_class.name = pci_lookup_name(pacc, namebuf, sizeof(namebuf), PCI_LOOKUP_CLASS, dev->device_class);

        // get vendor
        dev_info.vendor.id = dev->vendor_id;
        dev_info.vendor.name = pci_lookup_name(pacc, namebuf, sizeof(namebuf), PCI_LOOKUP_VENDOR, dev->vendor_id);

        // get device
        dev_info.device.id = dev->device_id;
        dev_info.device.name = pci_lookup_name(pacc, namebuf, sizeof(namebuf), PCI_LOOKUP_DEVICE, dev->vendor_id, dev->device_id);

#if PCI_LIB_VERSION < 0x030800
        // get sub_vendor
        dev_info.sub_vendor.id = find_sub_vendor_id(*dev);
        // get sub_device
        dev_info.sub_device.id = find_sub_device_id(*dev);
#else
        dev_info.sub_vendor.id = dev->subsys_vendor_id;
        dev_info.sub_device.id = dev->subsys_id;
#endif
        dev_info.sub_vendor.name = pci_lookup_name(pacc, namebuf, sizeof(namebuf), PCI_LOOKUP_VENDOR, dev_info.sub_vendor.id);
        dev_info.sub_device.name = pci_lookup_name(pacc, namebuf, sizeof(namebuf), PCI_LOOKUP_DEVICE, dev_info.sub_vendor.id,  dev_info.sub_device.id);

#if PCI_LIB_VERSION < 0x030800
        dev_info.s_driver = find_driver(*dev);
        dev_info.module_set = get_modules(*dev);
#else
        // get driver string
        const char* drv = pci_get_string_property(dev, PCI_FILL_DRIVER);
        dev_info.s_driver = drv ? drv : "";

        // get module string
        const char* mod = pci_get_string_property(dev, PCI_FILL_MODULE);
        std::string s_mod = mod ? mod : "";
        std::replace(s_mod.begin(), s_mod.end(), ',', ' ');
        std::istringstream iss(s_mod);
        str::string split_mod;
        while (iss >> split_mod) {
            dev_info.module_set.insert(split_mod);
        }
#endif
        
        if (device_type_match(dev_info, devices_type)) {
            dev_info.interfaces_set.clear();
            dev_info.b_ssh_if = false;
            dev_info.b_active = false;
            dev_info.numa_node = static_cast<int>(NUMANodeStatus::INIT);
            update_pci_device_netwrok_interface_name(dev_info);
            update_pci_device_numa_node(dev_info);

            if (devices_type == network_devices) {
                auto ssh_interface_set = get_ssh_interface_set();
                std::set<std::string> result;
                std::set_intersection(
                    ssh_interface_set.begin(), ssh_interface_set.end(),
                    dev_info.interfaces_set.begin(), dev_info.interfaces_set.end(),
                    std::inserter(result, result.begin())
                );
                
                if (!result.empty()) {
                    dev_info.b_ssh_if = true;
                    dev_info.b_active = true;
                }
            }

            dev_info.module_set.insert(dpdk_drivers.begin(), dpdk_drivers.end());
            dev_info.module_set.erase(dev_info.s_driver);

            devices[get_pci_address_string(dev_info.pci_address)] = dev_info;
        }
    }
    
    pci_cleanup(pacc);
}

void
update_pci_device_netwrok_interface_name(DeviceInfo& dev_info) {
    try {
        auto slot = get_pci_address_string(dev_info.pci_address);

        fs::path net_path = fs::path("/sys/bus/pci/devices") / slot / "net";
        if (fs::exists(net_path) && fs::is_directory(net_path)) {
            for (const auto& entry : fs::directory_iterator(net_path)) {
                dev_info.interfaces_set.insert(entry.path().filename().string());
            }
        }
    } catch (const std::exception& e) {
        // Handle filesystem errors if necessary
    }
}

void
update_pci_device_numa_node(DeviceInfo& dev_info) {
    try {
        auto slot = get_pci_address_string(dev_info.pci_address);

        fs::path numa_path = fs::path("/sys/bus/pci/devices") / slot / "numa_node";
        if (fs::exists(numa_path)) {
            std::ifstream numa_file(numa_path);
            if (numa_file.is_open()) {
                std::string s_numa_node;
                std::getline(numa_file, s_numa_node);
                dev_info.numa_node = std::stoi(s_numa_node);
            }
        }
    } catch (const std::exception& e) {
        // Handle filesystem errors if necessary
        dev_info.numa_node = static_cast<int>(NUMANodeStatus::ERROR);
    }
}

void
show_device_status(
    int device_group,
    std::string device_name,
    bool if_field
) {
    std::vector<std::map<std::string, std::set<uint16_t>>> devices_type;
    devices_type = device_group_map[device_group];

    std::vector<DeviceInfo> no_driver;
    std::vector<DeviceInfo> kernel_driver;
    std::vector<DeviceInfo> dpdk_driver;

    for (auto& dev_info_pair : devices) {
        DeviceInfo dev_info = dev_info_pair.second;
        if (device_type_match(dev_info, devices_type)) {
            if (dev_info.s_driver.empty()) {
                no_driver.push_back(dev_info);
            } else {
                bool is_dpdk_driver = false;
                for (auto& driver : dpdk_drivers) {
                    if (dev_info.interfaces_set.contains(driver)) {
                        is_dpdk_driver = true;
                        break;
                    }
                }

                if (is_dpdk_driver) {
                    dpdk_driver.push_back(dev_info);
                } else {
                    kernel_driver.push_back(dev_info);
                }
            }
        }
    }

    auto driver_count = no_driver.size() + kernel_driver.size() + dpdk_driver.size();

    if (driver_count == 0) {
        std::stringstream ss;
        ss << "No '" << device_name << "' devices detected";

        std::cout << std::endl;
        std::cout << ss.str() << std::endl;
        std::cout << std::string(ss.str().length(), '=') << std::endl;
        return;
    }

    if (dpdk_driver.size() > 0) {
        {
            std::stringstream ss;
            ss << device_name << " devices using DPDK-compatible driver";
            
            std::cout << std::endl;
            std::cout << ss.str() << std::endl;
            std::cout << std::string(ss.str().length(), '=') << std::endl;
        }
        
        for (auto& dev_info : dpdk_driver) {
            std::stringstream ss;

            ss << get_pci_address_string(dev_info.pci_address) << " ";
            ss << "'" << dev_info.device.name << " "
                << std::hex << std::setw(4) << std::setfill('0')
                << dev_info.device.id
                << std::dec << "' ";

            if (dev_info.numa_node >= 0) {
                ss << "numa_node=" << dev_info.numa_node << " ";
            }
            
            ss << "drv=" << dev_info.s_driver << " ";
            ss << "unused=";
            for (auto it = dev_info.module_set.begin(); it != dev_info.module_set.end(); it++) {
                ss << (it == dev_info.module_set.begin() ? "" : ",") << *it;
            }

            std::cout << ss.str() << std::endl;
        }
    }

    if (kernel_driver.size() > 0) {
        {
            std::stringstream ss;
            ss << device_name << " devices using kernel driver";
            
            std::cout << std::endl;
            std::cout << ss.str() << std::endl;
            std::cout << std::string(ss.str().length(), '=') << std::endl;
        }
        
        for (auto& dev_info : kernel_driver) {
            std::stringstream ss;

            ss << get_pci_address_string(dev_info.pci_address) << " ";
            ss << "'" << dev_info.device.name << " "
                << std::hex << std::setw(4) << std::setfill('0')
                << dev_info.device.id
                << std::dec << "' ";

            if (if_field) {
                ss << "if=";
                for (auto it = dev_info.interfaces_set.begin(); it != dev_info.interfaces_set.end(); it++) {
                    ss << (it == dev_info.interfaces_set.begin() ? "" : ",") << *it;
                }
                ss << " ";
            }

            if (dev_info.numa_node >= 0) {
                ss << "numa_node=" << dev_info.numa_node << " ";
            }
            
            ss << "drv=" << dev_info.s_driver << " ";
            ss << "unused=";
            for (auto it = dev_info.module_set.begin(); it != dev_info.module_set.end(); it++) {
                ss << (it == dev_info.module_set.begin() ? "" : ",") << *it;
            }

            if (dev_info.b_active) {
                ss << " *Active*";
            }

            std::cout << ss.str() << std::endl;
        }
    }

    if (no_driver.size() > 0) {
        {
            std::stringstream ss;
            ss << "Other " << device_name;
            
            std::cout << std::endl;
            std::cout << ss.str() << std::endl;
            std::cout << std::string(ss.str().length(), '=') << std::endl;
        }
        
        for (auto& dev_info : no_driver) {
            std::stringstream ss;

            ss << get_pci_address_string(dev_info.pci_address) << " ";
            ss << "'" << dev_info.device.name << " "
                << std::hex << std::setw(4) << std::setfill('0')
                << dev_info.device.id
                << std::dec << "' ";

            if (dev_info.numa_node >= 0) {
                ss << "numa_node=" << dev_info.numa_node << " ";
            }
            
            ss << "unused=";
            for (auto it = dev_info.module_set.begin(); it != dev_info.module_set.end(); it++) {
                ss << (it == dev_info.module_set.begin() ? "" : ",") << *it;
            }

            std::cout << ss.str() << std::endl;
        }
    }
}

bool
module_is_loaded(const std::string& module) {
    std::string target = module;

    // special case for vfio_pci (module is named vfio-pci,
    // but its .ko is named vfio_pci)
    if (module == "vfio_pci") {
        target = "vfio-pci";
    }

    fs::path sysfs_path = fs::path("/sys/module/");

    std::vector<std::string> mods;
    if (fs::exists(sysfs_path) && fs::is_directory(sysfs_path)) {
        for (const auto& entry : fs::directory_iterator(sysfs_path)) {
            if (fs::is_directory(entry)) {
                std::string s_mod = entry.path().filename().string();
                if (s_mod == "vfio_pci") {
                    s_mod = "vfio-pci";
                }
                mods.push_back(s_mod);
            }
        }
    }

    utsname buffer;
    if (uname(&buffer) != 0) {
        std::cerr << "uname failed\n";
        return false;
    }

    std::string release = buffer.release;
    fs::path os_mod_path = fs::path("/lib/modules/") / release / "modules.builtin";
    if (fs::exists(os_mod_path)) {
        std::ifstream os_mods(os_mod_path);
        if (os_mods.is_open()) {
            std::string line_str;
            while (std::getline(os_mods, line_str)) {
                fs::path mod_path(line_str);
                if (mod_path.stem().string() == target) {
                    return true;
                }
            }
        }   
    }

    return false;
}

void
check_modules() {
    dpdk_drivers.clear();

    for (const auto& driver : all_dpdk_drivers) {
        if (module_is_loaded(driver)) {
            dpdk_drivers.push_back(driver);
        }
    }
}

#endif