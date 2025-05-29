#define TCP_SNIFFER_H

void startTcpSniffer();

void process_packet(const char* buffer, ssize_t size);