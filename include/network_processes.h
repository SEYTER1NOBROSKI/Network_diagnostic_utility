#define NETWORK_UTILS_H

#include <string>

std::string hexToIp(const std::string& hex, bool ipv6 = false);

std::string tcp_State(const std::string& st);

int hex_ToPort(const std::string& hex);

bool isNumeric(const std::string& str);

std::string readSymlink(const std::string& path);

std::string findProcessByInode(const std::string& inode);

std::string formatAddress(const std::string& ip, int port, bool ipv6);

void parseConnections(const std::string& path, bool ipv6 = false, const std::string& protocol = "TCP");