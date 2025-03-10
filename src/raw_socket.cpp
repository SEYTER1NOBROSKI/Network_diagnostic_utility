#include "raw_socket.h"
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <netinet/ip_icmp.h>

#define TIMEOUT_SEC 2

unsigned short checksum(void *b, int len) {
    unsigned short *buf = (unsigned short *)b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2) {
        sum += *buf++;
    }

    if (len == 1) {
        sum += *(unsigned char *)buf;
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);

    result = ~sum;
    return result;
}

int createRawSocket(const std::string& interface) {
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }

    struct timeval timeout = { TIMEOUT_SEC, 0 };
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("setsockopt");
        close(sockfd);
        return -1;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, interface.c_str(), interface.size()) < 0) {
        perror("setsockopt - SO_BINDTODEVICE");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

bool sendICMP(int sockfd, struct sockaddr_in& dest_addr, struct icmphdr& icmp_hdr) {
    icmp_hdr.checksum = 0;
    icmp_hdr.checksum = checksum(&icmp_hdr, sizeof(icmp_hdr));

    ssize_t sent_bytes = sendto(sockfd, &icmp_hdr, sizeof(icmp_hdr), 0,
                                (struct sockaddr*)&dest_addr, sizeof(dest_addr));
    if (sent_bytes < 0) {
        perror("sendto");
        return false;
    }
    return true;
}

bool receiveICMP(int sockfd, struct sockaddr_in& sender) {
    char buffer[512];
    socklen_t sender_len = sizeof(sender);

    ssize_t received_bytes = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                                      (struct sockaddr*)&sender, &sender_len);
    if (received_bytes < 0) {
        perror("Error receiving ICMP response");
        return false;
    }

    std::cout << "Received " << received_bytes << " bytes from " << inet_ntoa(sender.sin_addr) << std::endl;
    return true;
}