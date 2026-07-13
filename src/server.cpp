#include "server.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

Server::Server(int port) : port_(port), server_fd_(-1) {}

std::string Server::handle_command(const std::vector<std::string>& tokens) {
    if (tokens.empty()) return resp_error("ERR empty command");

    std::string cmd = tokens[0];
    for (auto& c : cmd) c = toupper(c);

    if (cmd == "SET") {
        if (tokens.size() != 3) return resp_error("ERR wrong number of arguments for 'set'");
        store_.set(tokens[1], tokens[2]);
        return resp_simple_string("OK");
    } else if (cmd == "GET") {
        if (tokens.size() != 2) return resp_error("ERR wrong number of arguments for 'get'");
        auto val = store_.get(tokens[1]);
        if (!val) return resp_nil();
        return resp_bulk_string(*val);
    } else if (cmd == "DEL") {
        if (tokens.size() != 2) return resp_error("ERR wrong number of arguments for 'del'");
        bool deleted = store_.del(tokens[1]);
        return resp_integer(deleted ? 1 : 0);
    } else if (cmd == "PING") {
        return resp_simple_string("PONG");
    } else {
        return resp_error("ERR unknown command '" + cmd + "'");
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

        std::string buffer;
        char chunk[1024];
        while (true) {
            ssize_t bytes_read = read(client_fd, chunk, sizeof(chunk));
            if (bytes_read <= 0) {
                std::cout << "Client disconnected\n";
                break;
            }
            buffer.append(chunk, bytes_read);

            // Keep parsing as many complete commands as are in the buffer
            while (auto tokens = parse_resp_command(buffer)) {
                std::string response = handle_command(*tokens);
                write(client_fd, response.c_str(), response.size());
            }
        }

        close(client_fd);
    }
}