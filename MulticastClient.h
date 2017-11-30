//
// Created by Björn Hjortsberg on 18/11/17.
//

#pragma once

#include <string>

class MulticastClient
{
public:
    MulticastClient();
    MulticastClient(const std::string&);
    ~MulticastClient();

    void join(const std::string & addr);
    void leave(const std::string & addr);
    void listen(uint16_t port);

private:
    int sock_fd;
    bool mJoined;
    std::string mAddr;
};
