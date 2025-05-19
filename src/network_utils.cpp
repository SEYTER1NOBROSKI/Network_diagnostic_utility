#include "network_utils.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <iomanip>

void dnsResolveAndPrint(const std::string& input) {
    struct in_addr addr;
    if (inet_pton(AF_INET, input.c_str(), &addr) == 1) {
        // reverse lookup
        std::string hostname = resolveHostname(input);
        std::cout << "Reverse DNS lookup:\n";
        std::cout << "IP Address: " << input << "\n";
        std::cout << "Hostname: " << hostname << "\n";
    } else {
        // forward lookup
        struct sockaddr_in resolved = resolveIPAddress(input);
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(resolved.sin_addr), ip, INET_ADDRSTRLEN);

        std::cout << "Forward DNS lookup:\n";
        std::cout << "Hostname: " << input << "\n";
        std::cout << "IP Address: " << ip << "\n";
    }
}

std::string hexToIP(const std::string& hex) {
    unsigned int ip;
    std::stringstream ss;
    ss << std::hex << hex;
    ss >> ip;

    unsigned char bytes[4];
    bytes[0] = ip & 0xFF;
    bytes[1] = (ip >> 8) & 0xFF;
    bytes[2] = (ip >> 16) & 0xFF;
    bytes[3] = (ip >> 24) & 0xFF;

    std::ostringstream result;
    result << (int)bytes[0] << "."
           << (int)bytes[1] << "."
           << (int)bytes[2] << "."
           << (int)bytes[3];
    return result.str();
}

void printRoutingTable() {
    std::ifstream routeFile("/proc/net/route");
    std::string line;

    if (!routeFile.is_open()) {
        std::cerr << "Cannot open /proc/net/route\n";
        return;
    }

    std::getline(routeFile, line);

    std::cout << std::left
              << std::setw(10) << "Iface"
              << std::setw(20) << "Destination"
              << std::setw(20) << "Gateway" << "\n";
    std::cout << std::string(50, '-') << "\n";

    while (std::getline(routeFile, line)) {
        std::istringstream iss(line);
        std::string iface, destinationHex, gatewayHex;
        iss >> iface >> destinationHex >> gatewayHex;

        std::cout << std::left
                  << std::setw(10) << iface
                  << std::setw(20) << hexToIP(destinationHex)
                  << std::setw(20) << hexToIP(gatewayHex)
                  << "\n";
    }

    routeFile.close();
}

bool interfaceHasInternetAccess(const std::string& interface) {
    std::string cmd = "ping -I " + interface + " -c 1 -W 1 8.8.8.8 > /dev/null 2>&1";
    int result = system(cmd.c_str());
    return (result == 0);
}

std::string resolveHostname(const std::string& ip) {
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &sa.sin_addr);

    char host[NI_MAXHOST];
    if (getnameinfo((struct sockaddr*)&sa, sizeof(sa), host, sizeof(host), NULL, 0, NI_NAMEREQD) == 0) {
        return std::string(host);
    } else {
        return ip;
    }
}

struct sockaddr_in resolveIPAddress(const std::string& hostname) {
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;

    struct hostent* host = gethostbyname(hostname.c_str());
    if (host) {
        addr.sin_addr = *reinterpret_cast<struct in_addr*>(host->h_addr);
    } else {
        inet_pton(AF_INET, hostname.c_str(), &addr.sin_addr);
    }

    return addr;
}