#include "tracerout.h"
#include "tcp_sniffer.h"
#include "network_processes.h"
#include "network_utils.h"
#include <iostream>
#include <string>

void printHelp(const std::string& programName) {
    std::cout << "Usage:\n"
              << "  " << programName << " tcp_sniffer\n"
              << "      -> Runs a sniffer to intercept TCP packets on the network\n\n"
              
              << "  " << programName << " net-procs\n"
              << "      -> Displays a list of network processes and their connections\n\n"
              
              << "  " << programName << " connections\n"
              << "      -> Shows all active network connections\n\n"
              
              << "  " << programName << " arp -n <ip_addr>\n"
              << "      -> Shows the MAC address for the given IP address (without reverse DNS)\n\n"
              
              << "  " << programName << " arp\n"
              << "      -> Displays the current ARP table\n\n"
              
              << "  " << programName << " whois <ip_or_domain>\n"
              << "      -> Gets WHOIS information for an IP address or domain\n\n"
              
              << "  " << programName << " netstats <interface> <time>\n"
              << "      -> Shows statistics (how many bytes were transmitted/received and the data transfer rate) for interface <interface> for <time> seconds\n\n"
              
              << "  " << programName << " dns <hostname_or_ip>\n"
              << "      -> Performs DNS resolution for a hostname or IP address\n\n"
              
              << "  " << programName << " route\n"
              << "      -> Displays the route table\n\n"
              
              << "  " << programName << " <hostname> <interface>\n"
              << "      -> Determines the path to the node (Traceroute) via the specified interface\n";
}

int main(int argc, char* argv[]) {

    if (argc < 2) {
        std::cerr << "Usage:\n"
                  << "  " << argv[0] << " tcp_sniffer\n"
                  << "  " << argv[0] << " net-procs\n"
                  << "  " << argv[0] << " connections\n"
                  << "  " << argv[0] << " arp -n <ip_addr>\n"
                  << "  " << argv[0] << " arp\n"
                  << "  " << argv[0] << " whois <ip_or_domain>\n"
                  << "  " << argv[0] << " netstats <interface> <time>\n"
                  << "  " << argv[0] << " dns <hostname_or_ip>\n"
                  << "  " << argv[0] << " route\n"
                  << "  " << argv[0] << " <hostname> <interface>\n";
        return 1;
    }

    std::string command = argv[1];

    if (command == "help") {
        printHelp(argv[0]);
        return 0;
    }

    if (command == "tcp_sniffer") {
        startTcpSniffer();
        return 0;
    }

    if (command == "net-procs") {
        parseConnections("/proc/net/tcp", false, "TCP");
        parseConnections("/proc/net/udp", false, "UDP");
        parseConnections("/proc/net/tcp6", true, "TCP");
        parseConnections("/proc/net/udp6", true, "UDP");
        return 0;
    }

    if (command == "connections") {
        showAllConnections();
        return 0;
    }
    
    if (command == "arp" && argc >= 4 && std::string(argv[2]) == "-n") {
        printARPTable(argv[3]);
        return 0;
    }

    if (command == "arp") {
        printARPTable();
        return 0;
    }

    if (command == "whois") {
        if (argc < 3) {
            std::cerr << "Usage: " << argv[0] << " whois <ip_or_domain>\n";
            return 1;
        }

        std::string result = whoisLookup(std::string(argv[2]));
        std::cout << result << std::endl;
        return 0;
    }

    if (command == "netstats") {
        if (argc < 4) {
            std::cerr << "Usage: " << argv[0] << " netstats <interface> <seconds>\n";
            return 1;
        }
        std::string interface = argv[2];
        int seconds = std::atoi(argv[3]);
        monitorNetworkInterface(interface, seconds);
        return 0;
    }


    if (command == "route") {
        printRoutingTable();
        return 0;
    }

    if (command == "dns") {
        if (argc < 3) {
            std::cerr << "Usage: " << argv[0] << " dns <hostname_or_ip>\n";
            return 1;
        }
        dnsResolveAndPrint(argv[2]);
        return 0;
    }

    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <hostname> <interface>\n";
        return 1;
    }

    std::string hostname = argv[1];
    std::string interface = argv[2];

    std::cout << "Your Network Interface: " << interface << "\n";

    if (runTraceroute(hostname, interface)) {
        std::cout << "Traceroute completed.\n";
    } else {
        std::cerr << "Traceroute failed.\n";
    }

    return 0;
}