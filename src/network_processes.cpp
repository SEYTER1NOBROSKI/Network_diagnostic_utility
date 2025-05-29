#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <vector>
#include <map>
#include <regex>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstring>
#include <arpa/inet.h>

std::string tcp_State(const std::string& st) {
    static std::map<std::string, std::string> states = {
        {"01", "ESTABLISHED"}, {"02", "SYN_SENT"}, {"03", "SYN_RECV"}, {"04", "FIN_WAIT1"},
        {"05", "FIN_WAIT2"}, {"06", "TIME_WAIT"}, {"07", "CLOSE"}, {"08", "CLOSE_WAIT"},
        {"09", "LAST_ACK"}, {"0A", "LISTEN"}, {"0B", "CLOSING"}
    };
    return states.count(st) ? states[st] : "UNKNOWN";
}

std::string hexToIp(const std::string& hex, bool ipv6 = false) {
    if (!ipv6) {
        unsigned int ip[4];
        sscanf(hex.c_str(), "%2X%2X%2X%2X", &ip[3], &ip[2], &ip[1], &ip[0]);
        std::stringstream ss;
        ss << ip[0] << "." << ip[1] << "." << ip[2] << "." << ip[3];
        return ss.str();
    } else {
        if (hex.size() != 32) return "Invalid IPv6";

        unsigned char bytes[16];
        for (int i = 0; i < 16; ++i) {
            std::string byteStr = hex.substr(i * 2, 2);
            unsigned int byteVal;
            sscanf(byteStr.c_str(), "%02X", &byteVal);
            bytes[15 - i] = static_cast<unsigned char>(byteVal);
        }

        char str[INET6_ADDRSTRLEN];
        if (inet_ntop(AF_INET6, bytes, str, INET6_ADDRSTRLEN) == nullptr) {
            return "Invalid IPv6";
        }
        return std::string(str);
    }
}

int hex_ToPort(const std::string& hex) {
    int port;
    sscanf(hex.c_str(), "%X", &port);
    return port;
}

bool isNumeric(const std::string& str) {
    return all_of(str.begin(), str.end(), ::isdigit);
}

std::string readSymlink(const std::string& path) {
    char buf[PATH_MAX];
    ssize_t len = readlink(path.c_str(), buf, sizeof(buf)-1);
    if (len != -1) {
        buf[len] = '\0';
        return std::string(buf);
    }
    return "";
}

std::string findProcessByInode(const std::string& inode) {
    DIR* proc = opendir("/proc");
    if (!proc) return "unknown";

    struct dirent* entry;
    while ((entry = readdir(proc)) != nullptr) {
        if (entry->d_type != DT_DIR) continue;
        std::string pid = entry->d_name;
        if (!isNumeric(pid)) continue;

        std::string fd_path = "/proc/" + pid + "/fd";
        DIR* fd_dir = opendir(fd_path.c_str());
        if (!fd_dir) continue;

        struct dirent* fd_entry;
        while ((fd_entry = readdir(fd_dir)) != nullptr) {
            if (fd_entry->d_name[0] == '.') continue;

            std::string full_fd = fd_path + "/" + fd_entry->d_name;
            std::string target = readSymlink(full_fd);
            if (target.find("socket:[") != std::string::npos) {
                size_t start = target.find("[") + 1;
                size_t end = target.find("]");
                if (inode == target.substr(start, end - start)) {
                    std::ifstream cmd("/proc/" + pid + "/comm");
                    std::string process;
                    getline(cmd, process);
                    closedir(fd_dir);
                    closedir(proc);
                    return process + " (PID " + pid + ")";
                }
            }
        }
        closedir(fd_dir);
    }

    closedir(proc);
    return "unknown";
}

std::string formatAddress(const std::string& ip, int port, bool ipv6) {
    if (port == 0) return ip;
    if (ipv6) {
        return "[" + ip + "]:" + std::to_string(port);
    } else {
        return ip + ":" + std::to_string(port);
    }
}

void parseConnections(const std::string& path, bool ipv6, const std::string& protocol) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << path << std::endl;
        return;
    }

    std::string line;
    std::getline(file, line);

    const int procWidth = 26;
    const int addrWidth = 28;
    const int stateWidth = 15;
    const int protoWidth = 10;

    std::cout << std::left
         << std::setw(procWidth) << "Process (PID)"
         << std::setw(addrWidth) << "Local Address"
         << std::setw(addrWidth) << "Remote Address"
         << std::setw(stateWidth) << "State"
         << std::setw(protoWidth) << "Protocol"
         << std::endl;

    std::cout << std::string(procWidth + 2 * addrWidth + stateWidth + protoWidth, '-') << std::endl;


    while (getline(file, line)) {
        std::istringstream iss(line);
        std::string sl, local_address, rem_address, st, tx_rx_queue, tr_tm_when, retrnsmt;
        std::string uid, timeout, inode;

        iss >> sl >> local_address >> rem_address >> st >> tx_rx_queue >> tr_tm_when >> retrnsmt >> uid >> timeout >> inode;

        size_t local_colon = local_address.find(':');
        size_t rem_colon = rem_address.find(':');

        std::string local_ip = hexToIp(local_address.substr(0, local_colon), ipv6);
        int local_port = hex_ToPort(local_address.substr(local_colon + 1));

        std::string rem_ip = hexToIp(rem_address.substr(0, rem_colon), ipv6);
        int rem_port = hex_ToPort(rem_address.substr(rem_colon + 1));

        std::string state;
        if (protocol == "TCP") {
            state = tcp_State(st);
        } else if (protocol == "UDP") {
            if (st == "07") state = "CLOSE";
            else if (st == "0A") state = "LISTEN";
            else state = "UNCONN";
        } else {
            state = "UNKNOWN";
        }

        std::string proc = findProcessByInode(inode);

        std::cout << std::left
             << std::setw(procWidth) << proc
             << std::setw(addrWidth) << formatAddress(local_ip, local_port, ipv6)
             << std::setw(addrWidth) << formatAddress(rem_ip, rem_port, ipv6)
             << std::setw(stateWidth) << state
             << std::setw(protoWidth) << protocol
             << std::endl;
    }
}