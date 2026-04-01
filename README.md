# dpdk-devbind-cpp
A high-performance C++17 library and CLI tool for managing PCI device bindings, providing a native alternative to DPDK's dpdk-devbind.py.

# depend on
- Rocky
```
dnf config-manager --set-enabled crb
dnf install -y pciutils-devel kmod-devel
```

# build
```
cmake .
make
```

# run
- Print the current status of all known devices.
```
./app/dpdk-devbind -s
./app/dpdk-devbind --status
```
