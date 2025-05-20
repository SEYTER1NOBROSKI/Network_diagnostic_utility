#define NETWORK_UTILS_H

#include <string>
#include <netinet/in.h>

std::string queryWhoisServer(const std::string& server, const std::string& query);

std::string trim(const std::string& s);

std::string toLower(const std::string& s);

std::string whoisLookup (const std::string& query, const std::string& server = "whois.iana.org", int depth = 0, int maxDepth = 3);

void monitorNetworkInterface(const std::string& interface, int durationSeconds);

void dnsResolveAndPrint(const std::string& input);

std::string hexToIP(const std::string& hex);

void printRoutingTable();

bool interfaceHasInternetAccess(const std::string& interface);

std::string resolveHostname(const std::string& ip);

struct sockaddr_in resolveIPAddress(const std::string& hostname);
