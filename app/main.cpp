#include "dpdk_devbind.h"

#include <iostream>
#include <getopt.h>

int main(int argc, char* argv[]) {
    int opt;

    bool status_flag = false;

    static option long_options[] = {
        {"status", no_argument, 0, 's'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "s", long_options, nullptr)) != -1) {
        switch (opt) {
            case 's':
                status_flag = true;
                break;
        }
    }

    if (status_flag) {
        show_status();
    } else {
        std::cout << "Error: No action specified for devices. Please give a --bind, --ubind or --status option" << std::endl;
        std::cout << "usage: dpdk-devbind [-h] [-s] [--status-dev {baseband,compress,crypto,dma,event,mempool,misc,net,regex,ml}] [-b DRIVER | -u] [--noiommu-mode] [--force] [DEVICE ...]" << std::endl;
    }

    return 0;
}
