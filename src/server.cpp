#include "server.hpp"
#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

Server::Server(int port) : port_(port), server_fd_(-1) {}

std::string Server::handle_command(const std::string& line) {
    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;

    if (cmd == "SET") {
        std::string key, value;
        iss >> key >> value;
        if (key.empty() || value.empty()) return "ERR wrong number of arguments\n";
        store_.set(key, value);
        return "OK\n";
    } else if (cmd == "GET") {
        std::string key;
        iss >> key;
        if (key.empty()) return "ERR wrong number of arguments\n";
        auto val = store_.get(key);
        if (!val) return "(nil)\n";
        return *val + "\n";
    } else if (cmd == "DEL") {
        std::string key;
        iss >> key;
        if (key.empty()) return "ERR wrong number of arguments\n";
        bool deleted = store_.del(key);
        return (deleted ? "1" : "0") + std::string("\n");
    } else {
        return "ERR unknown command\n";
    }
}

void Server::run() {
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        std::cerr << "Failed to create socket\n";
        return;
    }

    int opt = 1;
    setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port_);

    if (bind(server_fd_, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Bind failed on port " << port_ << "\n";
        return;
    }

    if (listen(server_fd_, 10) < 0) {
        std::cerr << "Listen failed\n";
        return;
    }

    std::cout << "mini_redis listening on port " << port_ << "\n";

    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd_, (sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            std::cerr << "Accept failed\n";
            continue;
        }

        std::cout << "Client connected\n";

        std::string leftover;
        char buffer[1024];
        while (true) {
            ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
            if (bytes_read <= 0) {
                std::cout << "Client disconnected\n";
                break;
            }
            buffer[bytes_read] = '\0';
            leftover += buffer;

            size_t pos;
            while ((pos = leftover.find('\n')) != std::string::npos) {
                std::string line = leftover.substr(0, pos);
                if (!line.empty() && line.back() == '\r') line.pop_back(); // handle CRLF
                leftover.erase(0, pos + 1);

                std::string response = handle_command(line);
                write(client_fd, response.c_str(), response.size());
            }
        }

        close(client_fd);
    }
}