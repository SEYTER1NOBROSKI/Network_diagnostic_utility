#define NETWORK_UTILS_H

#include <string>
#include <netinet/in.h>

void dnsResolveAndPrint(const std::string& input);

std::string hexToIP(const std::string& hex);

void printRoutingTable();

bool interfaceHasInternetAccess(const std::string& interface);

std::string resolveHostname(const std::string& ip);

struct sockaddr_in resolveIPAddress(const std::string& hostname);
