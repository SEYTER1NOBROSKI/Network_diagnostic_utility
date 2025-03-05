#include "network_utils.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <cstring>
#include <iostream>

std::string ipToString(struct sockaddr_in addr) {
    return inet_ntoa(addr.sin_addr);
}

std::string getHostName(const std::string& ip) {
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

std::string getHostByAddr(const std::string& ip) {
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