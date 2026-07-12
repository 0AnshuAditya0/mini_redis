#pragma once

class Server {
public:
    Server(int port);
    void run();

private:
    int port_;
    int server_fd_;
};