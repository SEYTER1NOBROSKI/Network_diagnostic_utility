#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <dirent.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <iomanip>
#include <cctype>
#include <limits.h>
#include <algorithm>

std::string hexToIpPort(const std::string& hexIpPort) {
    size_t colon_pos = hexIpPort.find(':');
    if (colon_pos == std::string::npos)
        return hexIpPort;

    std::string hexIp = hexIpPort.substr(0, colon_pos);
    std::string hexPort = hexIpPort.substr(colon_pos + 1);

    if (hexIp.size() != 8)
        return hexIpPort;

    unsigned int b1 = std::stoi(hexIp.substr(6, 2), nullptr, 16);
    unsigned int b2 = std::stoi(hexIp.substr(4, 2), nullptr, 16);
    unsigned int b3 = std::stoi(hexIp.substr(2, 2), nullptr, 16);
    unsigned int b4 = std::stoi(hexIp.substr(0, 2), nullptr, 16);

    std::string ip = std::to_string(b1) + "." + std::to_string(b2) + "." + std::to_string(b3) + "." + std::to_string(b4);
    unsigned int port = std::stoi(hexPort, nullptr, 16);

    return ip + ":" + std::to_string(port);
}

std::string findPidByInode(const std::string& inode) {
    DIR* proc = opendir("/proc");
    if (!proc) {
        perror("opendir /proc");
        return "";
    }

    struct dirent* pid_entry;
    while ((pid_entry = readdir(proc)) != nullptr) {
        // Пропускаємо все, що не є числом (PID)
        if (!std::all_of(pid_entry->d_name, pid_entry->d_name + std::strlen(pid_entry->d_name), ::isdigit))
            continue;

        std::string pid = pid_entry->d_name;
        std::string fd_dir = "/proc/" + pid + "/fd";
        DIR* fd = opendir(fd_dir.c_str());
        if (!fd) continue;  // Можливо, нема прав або процес завершився

        struct dirent* fd_entry;
        while ((fd_entry = readdir(fd)) != nullptr) {
            if (strcmp(fd_entry->d_name, ".") == 0 || strcmp(fd_entry->d_name, "..") == 0)
                continue;

            std::string fd_path = fd_dir + "/" + fd_entry->d_name;
            char link_target[PATH_MAX];
            ssize_t len = readlink(fd_path.c_str(), link_target, sizeof(link_target) - 1);
            if (len != -1) {
                link_target[len] = '\0';
                std::string link_str(link_target);
                if (link_str.find("socket:[" + inode + "]") != std::string::npos) {
                    closedir(fd);
                    closedir(proc);
                    return pid;
                }
            }
        }
        closedir(fd);
    }
    closedir(proc);
    return "";
}

std::string getProcessName(const std::string& pid) {
    if (pid.empty()) {
        return "-";
    }
    std::string commPath = "/proc/" + pid + "/comm";
    std::ifstream commFile(commPath);
    if (!commFile.is_open()) {
        std::cerr << "Cannot open " << commPath << std::endl;
        return "-";
    }

    std::string name;
    std::getline(commFile, name);
    return name;
}

std::string getProcessNameByPid(const std::string& pid) {
    std::ifstream comm_file("/proc/" + pid + "/comm");
    std::string name;
    if (comm_file.is_open()) std::getline(comm_file, name);
    return name;
}

std::string tcp_State(const std::string& st) {
    static const std::map<std::string, std::string> states = {
        {"01", "ESTABLISHED"}, {"02", "SYN_SENT"},     {"03", "SYN_RECV"},
        {"04", "FIN_WAIT1"},   {"05", "FIN_WAIT2"},    {"06", "TIME_WAIT"},
        {"07", "CLOSE"},       {"08", "CLOSE_WAIT"},   {"09", "LAST_ACK"},
        {"0A", "LISTEN"},      {"0B", "CLOSING"},      {"0C", "NEW_SYN_RECV"}
    };
    auto it = states.find(st);
    return it != states.end() ? it->second : "UNKNOWN";
}

void printNetworkProcesses(const std::string& proto, const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Cannot open " << path << std::endl;
        return;
    }

    std::string line;
    std::getline(file, line);  // Пропускаємо заголовок

    std::cout << std::left
            << std::setw(6)  << "Proto"
            << std::setw(23) << "Local Address"
            << std::setw(23) << "Foreign Address"
            << std::setw(13) << "State"
            << "PID/Program name\n";


    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string sl, local_address, rem_address, st, queue, tr, tm_when, retrnsmt, uid, timeout, inode;

        if (!(iss >> sl >> local_address >> rem_address >> st >> queue >> tr >> tm_when >> retrnsmt >> uid >> timeout >> inode))
            continue;

        std::string state = tcp_State(st);
        if (state != "ESTABLISHED") continue;

        size_t colonPos = queue.find(':');
        std::string recvQ = colonPos != std::string::npos ? queue.substr(0, colonPos) : "0";
        std::string sendQ = colonPos != std::string::npos ? queue.substr(colonPos + 1) : "0";

        std::string local = hexToIpPort(local_address);
        std::string remote = hexToIpPort(rem_address);

        std::string pid = findPidByInode(inode);
        std::string procName = pid.empty() ? "-" : getProcessName(pid);
        std::string outputProc = procName.empty() ? "-" : procName;

        std::cout << std::left
                << std::setw(6)  << proto
                << std::setw(23) << local
                << std::setw(23) << remote
                << std::setw(13) << state
                << outputProc << "\n";

    }
}