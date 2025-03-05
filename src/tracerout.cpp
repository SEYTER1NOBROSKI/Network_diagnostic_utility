#include "tracerout.h"
#include "network_utils.h"
#include "raw_socket.h"
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAX_HOPS 30

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

bool runTraceroute(const std::string& hostname) {
    int sockfd = createRawSocket();
    if (sockfd < 0) return false;

    struct sockaddr_in dest_addr = resolveIPAddress(hostname);
    if (dest_addr.sin_addr.s_addr == INADDR_NONE) {
        std::cerr << "Cannot resolve hostname: " << hostname << "\n";
        return false;
    }

    for (int ttl = 1; ttl <= MAX_HOPS; ttl++) {
        setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));

        struct icmphdr icmp_hdr = {};
        icmp_hdr.type = ICMP_ECHO;
        icmp_hdr.code = 0;
        icmp_hdr.un.echo.id = getpid();
        icmp_hdr.un.echo.sequence = ttl;
        icmp_hdr.checksum = 0;

        if (!sendICMP(sockfd, dest_addr, icmp_hdr)) {
            std::cerr << "Failed to send ICMP packet at hop " << ttl << "\n";
            break;
        }

        struct sockaddr_in sender;
        std::cout << "Waiting for ICMP response...\n";
        if (receiveICMP(sockfd, sender)) {
            std::string senderIP = inet_ntoa(sender.sin_addr);
            std::string hostname = resolveHostname(senderIP);
            std::cout << "Received response from: " << senderIP << std::endl;

            std::cout << ttl << " " << senderIP << " (" << hostname << ")\n";

            if (sender.sin_addr.s_addr == dest_addr.sin_addr.s_addr) {
                std::cout << "Destination reached!\n";
                break;
            }
        } else {
            std::cout << ttl << " * * *\n";
        }

        usleep(500000);
    }

    close(sockfd);
    return true;
}