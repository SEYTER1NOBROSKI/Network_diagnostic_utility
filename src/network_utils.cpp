#include "network_utils.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cerrno>
#include <algorithm>
#include <stdexcept>

void printARPTable(const std::string& targetIP) {
    std::ifstream ARPFile("/proc/net/arp");

    if (!ARPFile.is_open()) {
        std::cerr << "Cannot open /proc/net/arp\n";
        return;
    }

    std::string line;
    std::getline(ARPFile, line);

    bool found = false;

    std::cout << std::left
              << std::setw(20) << "IP Address"
              << std::setw(20) << "MAC Address"
              << std::setw(10) << "Interface" << "\n";
    std::cout << std::string(50, '-') << "\n";

    while (std::getline(ARPFile, line)) {
        std::istringstream iss(line);
        std::string ip, hwType, flags, mac, mask, device;

        if (iss >> ip >> hwType >> flags >> mac >> mask >> device) {
            if (targetIP.empty() || ip == targetIP) {
                std::cout << std::left
                          << std::setw(20) << ip
                          << std::setw(20) << mac
                          << std::setw(10) << device << "\n";
                found = true;

                if (!targetIP.empty()) {
                    break;
                }
            }
        }
    }

    if (!found && !targetIP.empty()) {
        std::cout << "No MAC entry found for IP: " << targetIP << "\n";
    }

    ARPFile.close();
}

std::string queryWhoisServer(const std::string& server, const std::string& query) {
    addrinfo hints{}, *res;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;

    if (getaddrinfo(server.c_str(), "43", &hints, &res) != 0) {
        throw std::runtime_error("Failed to resolve whois server: " + server);
    }

    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) {
        freeaddrinfo(res);
        throw std::runtime_error("Failed to create socket");
    }

    if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
        close(sock);
        freeaddrinfo(res);
        throw std::runtime_error("Failed to connect to whois server: " + server);
    }

    freeaddrinfo(res);

    std::string queryStr = query + "\r\n";
    send(sock, queryStr.c_str(), queryStr.size(), 0);

    std::string response;
    char buffer[4096];
    ssize_t bytesRead;

    while ((bytesRead = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        response.append(buffer, bytesRead);
    }

    close(sock);

    return response;
}

std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
    return s.substr(start, end - start);
}

std::string toLower(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::string whoisLookup(const std::string& query, const std::string& server, int depth, int maxDepth) {

    if (depth > maxDepth) {
        return "[Max recursion depth reached]\n";
    }

    std::string response;
    try {
        response = queryWhoisServer(server, query);
    } catch (const std::exception& e) {
        return std::string("[Error querying server ") + server + "]: " + e.what() + "\n";
    }

    const std::string prefixes[] = { "refer:", "registrar whois server:", "whois:" };
    std::string lowerResponse = toLower(response);

    for (const auto& prefix : prefixes) {
        size_t pos = lowerResponse.find(prefix);
        if (pos != std::string::npos) {
            size_t lineEnd = lowerResponse.find('\n', pos);
            if (lineEnd == std::string::npos) lineEnd = lowerResponse.size();

            std::string line = response.substr(pos, lineEnd - pos);

            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                std::string nextServer = trim(line.substr(colonPos + 1));
                if (!nextServer.empty() && nextServer != server) {
                    return response + "\n----- Redirected to: " + nextServer + " -----\n" +
                           whoisLookup(query, nextServer, depth + 1, maxDepth);
                }
            }
        }
    }

    return response;
}

void monitorNetworkInterface(const std::string& interface, int durationSeconds) {
    std::string speedPath = "/sys/class/net/" + interface + "/speed";
    std::string rxPath = "/sys/class/net/" + interface + "/statistics/rx_bytes";
    std::string txPath = "/sys/class/net/" + interface + "/statistics/tx_bytes";

    std::ifstream speedFile(speedPath);
    int speedMbps = -1;
    if (speedFile) {
        speedFile >> speedMbps;
        speedFile.close();
    }

    std::cout << "Interface: " << interface << "\n";
    if (speedMbps > 0)
        std::cout << "Maximum link speed of interface: " << speedMbps << " Mbps\n";
    else
        std::cout << "Maximum link speed of interface: Unknown\n";

    std::cout << "Monitoring RX/TX traffic for " << durationSeconds << " seconds...\n";

    auto readBytes = [](const std::string& path) -> long long {
        std::ifstream file(path);
        long long bytes = 0;
        file >> bytes;
        return bytes;
    };

    long long initialRx = readBytes(rxPath);
    long long initialTx = readBytes(txPath);

    sleep(durationSeconds);

    long long finalRx = readBytes(rxPath);
    long long finalTx = readBytes(txPath);

    long long rxDelta = finalRx - initialRx;
    long long txDelta = finalTx - initialTx;

    double rxRate = static_cast<double>(rxDelta) / durationSeconds; // bytes/sec
    double txRate = static_cast<double>(txDelta) / durationSeconds;

    // Convert to Mbps (1 byte = 8 bits, 1_000_000 bits = 1 Mbps)
    double rxRateMbps = rxRate * 8 / 1000000;
    double txRateMbps = txRate * 8 / 1000000;

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Received: " << rxDelta << " bytes (" << rxRateMbps << " Mbps)\n";
    std::cout << "Transmitted: " << txDelta << " bytes (" << txRateMbps << " Mbps)\n";
}

void dnsResolveAndPrint(const std::string& input) {
    struct in_addr addr;
    if (inet_pton(AF_INET, input.c_str(), &addr) == 1) {
        // reverse lookup
        std::string hostname = resolveHostname(input);
        std::cout << "Reverse DNS lookup:\n";
        std::cout << "IP Address: " << input << "\n";
        std::cout << "Hostname: " << hostname << "\n";
    } else {
        // forward lookup
        struct sockaddr_in resolved = resolveIPAddress(input);
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(resolved.sin_addr), ip, INET_ADDRSTRLEN);

        std::cout << "Forward DNS lookup:\n";
        std::cout << "Hostname: " << input << "\n";
        std::cout << "IP Address: " << ip << "\n";
    }
}

std::string hexToIP(const std::string& hex) {
    unsigned int ip;
    std::stringstream ss;
    ss << std::hex << hex;
    ss >> ip;

    unsigned char bytes[4];
    bytes[0] = ip & 0xFF;
    bytes[1] = (ip >> 8) & 0xFF;
    bytes[2] = (ip >> 16) & 0xFF;
    bytes[3] = (ip >> 24) & 0xFF;

    std::ostringstream result;
    result << (int)bytes[0] << "."
           << (int)bytes[1] << "."
           << (int)bytes[2] << "."
           << (int)bytes[3];
    return result.str();
}

void printRoutingTable() {
    std::ifstream routeFile("/proc/net/route");
    std::string line;

    if (!routeFile.is_open()) {
        std::cerr << "Cannot open /proc/net/route\n";
        return;
    }

    std::getline(routeFile, line);

    std::cout << std::left
              << std::setw(10) << "Iface"
              << std::setw(20) << "Destination"
              << std::setw(20) << "Gateway" << "\n";
    std::cout << std::string(50, '-') << "\n";

    while (std::getline(routeFile, line)) {
        std::istringstream iss(line);
        std::string interface, destinationHex, gatewayHex;
        iss >> interface >> destinationHex >> gatewayHex;

        std::cout << std::left
                  << std::setw(10) << interface
                  << std::setw(20) << hexToIP(destinationHex)
                  << std::setw(20) << hexToIP(gatewayHex)
                  << "\n";
    }

    routeFile.close();
}

bool interfaceHasInternetAccess(const std::string& interface) {
    std::string cmd = "ping -I " + interface + " -c 1 -W 1 8.8.8.8 > /dev/null 2>&1";
    int result = system(cmd.c_str());
    return (result == 0);
}

std::string resolveHostname(const std::string& ip) {
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &sa.sin_addr);

    char host[NI_MAXHOST];
    if (getnameinfo((struct sockaddr*)&sa, sizeof(sa), host, sizeof(host), NULL, 0, NI_NAMEREQD) == 0) {
        return std::string(host);
    } else {
        return ip;
    }
}

struct sockaddr_in resolveIPAddress(const std::string& hostname) {
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;

    struct hostent* host = gethostbyname(hostname.c_str());
    if (host) {
        addr.sin_addr = *reinterpret_cast<struct in_addr*>(host->h_addr);
    } else {
        inet_pton(AF_INET, hostname.c_str(), &addr.sin_addr);
    }

    return addr;
}