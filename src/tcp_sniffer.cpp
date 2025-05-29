#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "tcp_sniffer.h"

#define BUF_SIZE 2048

// IP header structure
struct ip_header {
    unsigned char  ihl:4, version:4;
    unsigned char  tos;
    unsigned short tot_len;
    unsigned short id;
    unsigned short frag_off;
    unsigned char  ttl;
    unsigned char  protocol;
    unsigned short check;
    struct in_addr ip_src, ip_dst;
};

// TCP header structure
struct tcp_header {
    unsigned short source_port;
    unsigned short dest_port;
    unsigned int seq_num;
    unsigned int ack_num;
    unsigned char data_offset:4, reserved:4;
    unsigned char flags;
    unsigned short window_size;
    unsigned short checksum;
    unsigned short urgent_pointer;
};

void process_packet(const char* buffer, ssize_t size) {
    const ip_header* ip = (const ip_header*)buffer;
    const tcp_header* tcp = (const tcp_header*)(buffer + sizeof(ip_header));

    std::cout << "Source IP: " << inet_ntoa(ip->ip_src)
              << " | Destination IP: " << inet_ntoa(ip->ip_dst)
              << " | Protocol: " << (int)ip->protocol
              << " | Packet Size: " << size << std::endl;

    std::cout << "Source Port: " << ntohs(tcp->source_port)
              << " | Destination Port: " << ntohs(tcp->dest_port)
              << " | Sequence Number: " << ntohl(tcp->seq_num)
              << " | Acknowledgment Number: " << ntohl(tcp->ack_num)
              << " | Flags: " << (int)tcp->flags << std::endl;
}

void startTcpSniffer() {
    int sockfd;
    char buffer[BUF_SIZE];

    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP)) < 0) {
        std::cerr << "Failed to create TCP socket!" << std::endl;
        return;
    }

    std::cout << "TCP listener started." << std::endl;

    while (true) {
        ssize_t n = recv(sockfd, buffer, BUF_SIZE, 0);
        if (n < 0) {
            std::cerr << "Error receiving packet!" << std::endl;
            continue;
        }

        process_packet(buffer, n);
    }

    close(sockfd);
}