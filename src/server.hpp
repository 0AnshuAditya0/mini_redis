#pragma once
#include "store.hpp"

class Server {
public:
    Server(int port);
    void run();

private:
    int port_;
    int server_fd_;
    Store store_;

    std::string handle_command(const std::string& line);
};