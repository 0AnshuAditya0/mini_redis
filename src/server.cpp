#include "server.hpp"
#include <iostream>
#include <cstring>
#include <thread>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

Server::Server(int port) : port_(port), server_fd_(-1) {}

std::string Server::encode_resp_array(const std::vector<std::string>& tokens) {
    std::string out = "*" + std::to_string(tokens.size()) + "\r\n";
    for (auto& t : tokens) {
        out += "$" + std::to_string(t.size()) + "\r\n" + t + "\r\n";
    }
    return out;
}

void Server::append_to_aof(const std::vector<std::string>& tokens) {
    std::lock_guard<std::mutex> lock(aof_mutex_);
    aof_file_ << encode_resp_array(tokens);
    aof_file_.flush();
}

void Server::load_aof() {
    std::ifstream in(aof_path_);
    if (!in.is_open()) return;

    std::string buffer((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    int replayed = 0;
    while (auto tokens = parse_resp_command(buffer)) {
        handle_command(*tokens, true);
        replayed++;
    }
    std::cout << "Replayed " << replayed << " commands from AOF\n";
}

std::string Server::handle_command(const std::vector<std::string>& tokens, bool from_aof) {
    if (tokens.empty()) return resp_error("ERR empty command");

    std::string cmd = tokens[0];
    for (auto& c : cmd) c = toupper(c);

    if (cmd == "SET") {
        if (tokens.size() != 3) return resp_error("ERR wrong number of arguments for 'set'");
        store_.set(tokens[1], tokens[2]);
        if (!from_aof) append_to_aof(tokens);
        return resp_simple_string("OK");
    } else if (cmd == "GET") {
        if (tokens.size() != 2) return resp_error("ERR wrong number of arguments for 'get'");
        auto val = store_.get(tokens[1]);
        if (!val) return resp_nil();
        return resp_bulk_string(*val);
    } else if (cmd == "DEL") {
        if (tokens.size() != 2) return resp_error("ERR wrong number of arguments for 'del'");
        bool deleted = store_.del(tokens[1]);
        if (!from_aof && deleted) append_to_aof(tokens);
        return resp_integer(deleted ? 1 : 0);
    } else if (cmd == "PING") {
        return resp_simple_string("PONG");
    } else {
        return resp_error("ERR unknown command '" + cmd + "'");
    }
}

void Server::handle_client(int client_fd) {
    std::cout << "Client connected (thread " << std::this_thread::get_id() << ")\n";

    std::string buffer;
    char chunk[1024];
    while (true) {
        ssize_t bytes_read = read(client_fd, chunk, sizeof(chunk));
        if (bytes_read <= 0) {
            std::cout << "Client disconnected\n";
            break;
        }
        buffer.append(chunk, bytes_read);

        while (auto tokens = parse_resp_command(buffer)) {
            std::string response = handle_command(*tokens);
            write(client_fd, response.c_str(), response.size());
        }
    }

    close(client_fd);
}

void Server::run() {
    load_aof();

    aof_file_.open(aof_path_, std::ios::app);
    if (!aof_file_.is_open()) {
        std::cerr << "Failed to open AOF file for writing\n";
        return;
    }

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

        // Spawn a thread per client, detach it — it cleans itself up on disconnect
        std::thread(&Server::handle_client, this, client_fd).detach();
    }
}