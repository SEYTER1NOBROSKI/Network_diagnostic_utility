#define NETWORK_UTILS_H

#include <string>

std::string hexToIpPort(const std::string& hexIpPort);

std::string findPidByInode(const std::string& inode);

std::string getProcessName(const std::string& pid);

std::string tcp_State(const std::string& st);

std::string getProcessNameByPid(const std::string& pid);

void printNetworkProcesses(const std::string& proto, const std::string& path);