#define RAW_SOCKET_H

#include <string>
#include <netinet/ip_icmp.h>
#include <netinet/in.h>

unsigned short checksum(void *b, int len);

int createRawSocket(const std::string& interface);

bool sendICMP(int sockfd, struct sockaddr_in& dest_addr, struct icmphdr& icmp_hdr);

bool receiveICMP(int sockfd, struct sockaddr_in& sender);
