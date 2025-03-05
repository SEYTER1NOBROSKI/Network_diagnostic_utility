#define NETWORK_UTILS_H

#include <string>
#include <netinet/in.h>

std::string ipToString(struct sockaddr_in addr);

std::string getHostName(const std::string& ip);

std::string getHostByAddr(const std::string& ip);
