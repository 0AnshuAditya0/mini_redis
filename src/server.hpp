#pragma once
#include "store.hpp"
#include "resp.hpp"
#include <vector>
#include <fstream>
#include <string>

class Server {
public:
    Server(int port);
    void run();

private:
    int port_;
    int server_fd_;
    Store store_;
    std::ofstream aof_file_;
    std::string aof_path_ = "appendonly.aof";

    std::string handle_command(const std::vector<std::string>& tokens, bool from_aof = false);
    void load_aof();
    void append_to_aof(const std::vector<std::string>& tokens);
    std::string encode_resp_array(const std::vector<std::string>& tokens);
};