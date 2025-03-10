#include "tracerout.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <hostname>\n";
        return 1;
    }

    std::string hostname = argv[1];
    std::string interface = argv[2];

    std::cout << "Your Network Interface: " << argv[2] << "\n";

    if (runTraceroute(hostname, interface)) {
        std::cout << "Traceroute completed successfully.\n";
    } else {
        std::cerr << "Traceroute failed.\n";
    }

    return 0;
}