#include "tracerout.h"

int main() {
    std::string hostname = "8.8.8.8";
    if (runTraceroute(hostname)) {
        std::cout << "Traceroute completed successfully.\n";
    } else {
        std::cerr << "Traceroute failed.\n";
    }
    return 0;
}