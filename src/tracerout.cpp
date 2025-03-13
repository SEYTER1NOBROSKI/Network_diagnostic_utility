#include "tracerout.h"
#include "network_utils.h"
#include "raw_socket.h"
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <chrono>

#define MAX_HOPS 30

bool runTraceroute(const std::string& hostname, const std::string& interface) {

    if (!interfaceHasInternetAccess(interface)) {
        std::cerr << "Error: Interface " << interface << " does not have internet access." << std::endl;
        return 1;
    }

    int sockfd = createRawSocket(interface);
    if (sockfd < 0) return false;

    struct sockaddr_in dest_addr = resolveIPAddress(hostname);
    if (dest_addr.sin_addr.s_addr == INADDR_NONE) {
        std::cerr << "Cannot resolve hostname: " << hostname << "\n";
        return false;
    }

    for (int ttl = 1; ttl <= MAX_HOPS; ttl++) {
        setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));

        std::cout << ttl << " ";

        struct sockaddr_in sender;
        std::vector<double> rtt_values;
        std::string senderIP;
        std::string senderHostname;

        bool received = false;

        for (int i = 0; i < 3; i++) {
            auto start_time = std::chrono::high_resolution_clock::now();

            struct icmphdr icmp_hdr = {};
            icmp_hdr.type = ICMP_ECHO;
            icmp_hdr.code = 0;
            icmp_hdr.un.echo.id = getpid();
            icmp_hdr.un.echo.sequence = ttl * 3 + i;
            icmp_hdr.checksum = 0;

            if (!sendICMP(sockfd, dest_addr, icmp_hdr)) {
                std::cerr << "Failed to send ICMP packet at hop " << ttl << "\n";
                continue;
            }

            if (receiveICMP(sockfd, sender)) {
                received = true;
                senderIP = inet_ntoa(sender.sin_addr);
                senderHostname = resolveHostname(senderIP);

                auto end_time = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double, std::milli> rtt = end_time - start_time;
                rtt_values.push_back(rtt.count());
            } else {
                rtt_values.push_back(-1.0);
            }
        }

        if (received) {
            std::cout << senderIP << " (" << senderHostname << ") ";
            for (double rtt : rtt_values) {
                if (rtt >= 0) {
                    std::cout << rtt << " ms ";
                } else {
                    std::cout << "* ";
                }
            }
            std::cout << "\n";
        } else {
            std::cout << "* * *\n";
        }

        if (sender.sin_addr.s_addr == dest_addr.sin_addr.s_addr) {
            std::cout << "Destination reached!\n";
            break;
        }
    }

    close(sockfd);
    return true;
}