#include "tracerout.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <hostname>\n";
        return 1;
    }

    std::string hostname = argv[1];

    if (runTraceroute(hostname)) {
        std::cout << "Traceroute completed successfully.\n";
    } else {
        std::cerr << "Traceroute failed.\n";
    }

    return 0;
}