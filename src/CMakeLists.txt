# import pkg-config tool
find_package(PkgConfig REQUIRED)

# find libpci
pkg_check_modules(PCI REQUIRED IMPORTED_TARGET libpci)
# find kmod
pkg_check_modules(KMOD REQUIRED IMPORTED_TARGET libkmod)

# compile libdpdk-devbind-c
add_library(dpdk-devbind-c STATIC
    dpdk_devbind.cpp
    dpdk_scan_driver.cpp
)

target_link_libraries(dpdk-devbind-c PUBLIC
    PkgConfig::PCI
    PkgConfig::KMOD
)

target_include_directories(dpdk-devbind-c PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
