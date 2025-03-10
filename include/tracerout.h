#define TRACEROUTE_H

#include <iostream>
#include <string>
#include <vector>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include "network_utils.h"

#define MAX_HOPS 30
#define TIMEOUT_MS 1000

bool runTraceroute(const std::string& target, const std::string& interface);

std::string resolveHostname(const std::string& ip);