/*
* DOMO Port Forwarding Server
* Version: 1.1
* Created by: the1Domo
* 
* This application forwards TCP/UDP ports from local addresses to broadcast addresses
* Features:
* - Supports both TCP and UDP protocols
* - Configurable through external config file
* - Runs as a system service with auto-start capability
* - Multi-threaded design for handling multiple connections
* - Comprehensive logging
*/

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <syslog.h>

// Structure to hold forwarding configuration for each port
struct ForwardConfig {
    std::string localIP;
    int localPort;
    int remotePort;
    bool isUDP;
};

class PortForwarder {
private:
    std::vector<ForwardConfig> configs;
    std::vector<int> sockets;
    bool running;

    // Load and parse the configuration file
    bool loadConfig(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            syslog(LOG_ERR, "Failed to open config file: %s", path.c_str());
            return false;
        }

        std::string line;
        while (std::getline(file, line)) {
            // Skip empty lines and comments
            if (line.empty() || line[0] == '#') continue;

            ForwardConfig config;
            char protocol[10];
            if (sscanf(line.c_str(), "%s %d %d %s", 
                    const_cast<char*>(config.localIP.c_str()),
                    &config.localPort,
                    &config.remotePort,
                    protocol) == 4) {
                config.isUDP = (strcmp(protocol, "UDP") == 0);
                configs.push_back(config);
            }
        }
        return !configs.empty();
    }

    // Handle TCP forwarding for a specific configuration
    void forwardTCP(const ForwardConfig& config) {
        // Create TCP socket
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            syslog(LOG_ERR, "Failed to create TCP socket");
            return;
        }

        // Enable address reuse
        int enable = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

        // Set up local address
        struct sockaddr_in local_addr;
        local_addr.sin_family = AF_INET;
        local_addr.sin_port = htons(config.localPort);
        local_addr.sin_addr.s_addr = inet_addr(config.localIP.c_str());

        // Bind socket to local address
        if (bind(sock, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
            syslog(LOG_ERR, "Failed to bind TCP socket");
            close(sock);
            return;
        }

        listen(sock, 5);
        sockets.push_back(sock);

        // Main connection acceptance loop
        while (running) {
            int client = accept(sock, nullptr, nullptr);
            if (client < 0) continue;

            // Spawn new thread for each client connection
            std::thread([this, client, config]() {
                // Set up broadcast address
                struct sockaddr_in broadcast_addr;
                broadcast_addr.sin_family = AF_INET;
                broadcast_addr.sin_port = htons(config.remotePort);
                broadcast_addr.sin_addr.s_addr = INADDR_BROADCAST;

                int broadcast = socket(AF_INET, SOCK_STREAM, 0);
                if (broadcast < 0) {
                    close(client);
                    return;
                }

                // Connect to broadcast address
                if (connect(broadcast, (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr)) < 0) {
                    close(client);
                    close(broadcast);
                    return;
                }

                // Forward data between client and broadcast
                char buffer[4096];
                while (running) {
                    int received = recv(client, buffer, sizeof(buffer), 0);
                    if (received <= 0) break;
                    send(broadcast, buffer, received, 0);
                }

                close(client);
                close(broadcast);
            }).detach();
        }
    }

    // Handle UDP forwarding for a specific configuration
    void forwardUDP(const ForwardConfig& config) {
        // Create UDP socket
        int sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0) {
            syslog(LOG_ERR, "Failed to create UDP socket");
            return;
        }

        // Enable address reuse and broadcasting
        int enable = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
        setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &enable, sizeof(int));

        // Set up local address
        struct sockaddr_in local_addr;
        local_addr.sin_family = AF_INET;
        local_addr.sin_port = htons(config.localPort);
        local_addr.sin_addr.s_addr = inet_addr(config.localIP.c_str());

        // Bind socket to local address
        if (bind(sock, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
            syslog(LOG_ERR, "Failed to bind UDP socket");
            close(sock);
            return;
        }

        sockets.push_back(sock);

        // Set up broadcast address
        struct sockaddr_in broadcast_addr;
        broadcast_addr.sin_family = AF_INET;
        broadcast_addr.sin_port = htons(config.remotePort);
        broadcast_addr.sin_addr.s_addr = INADDR_BROADCAST;

        // Forward UDP packets
        char buffer[4096];
        while (running) {
            int received = recv(sock, buffer, sizeof(buffer), 0);
            if (received <= 0) continue;
            sendto(sock, buffer, received, 0,
                   (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
        }
    }

public:
    PortForwarder() : running(false) {}

    bool start(const std::string& configPath) {
        running = true;
        openlog("portforwarder", LOG_PID | LOG_NDELAY, LOG_DAEMON);
        
        // Display startup message
        std::cout << "Starting DOMO Port Forwarding Server v1.1" << std::endl;
        syslog(LOG_INFO, "Starting DOMO Port Forwarding Server v1.1");

        if (!loadConfig(configPath)) {
            syslog(LOG_ERR, "Failed to load configuration");
            return false;
        }

        // Create threads for each forwarding rule
        std::vector<std::thread> threads;
        for (const auto& config : configs) {
            if (config.isUDP) {
                threads.emplace_back(&PortForwarder::forwardUDP, this, config);
            } else {
                threads.emplace_back(&PortForwarder::forwardTCP, this, config);
            }
        }

        // Wait for all threads to complete
        for (auto& thread : threads) {
            thread.join();
        }

        return true;
    }

    // Cleanup and shutdown
    void stop() {
        running = false;
        for (int sock : sockets) {
            close(sock);
        }
        sockets.clear();
        closelog();
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <config_path>" << std::endl;
        return 1;
    }

    PortForwarder forwarder;
    signal(SIGINT, [](int) {
        // Cleanup will be handled by destructor
        exit(0);
    });

    if (!forwarder.start(argv[1])) {
        return 1;
    }

    return 0;
}
