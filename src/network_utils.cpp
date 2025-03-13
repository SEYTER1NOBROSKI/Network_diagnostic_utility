#include "network_utils.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <cstring>
#include <iostream>
#include <unistd.h>

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