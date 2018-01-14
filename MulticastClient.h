//
// Created by Björn Hjortsberg on 18/11/17.
//

#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <functional>
#include <map>

class MulticastClient
{
public:
    MulticastClient();
    ~MulticastClient();

    void join(const std::string& addr, uint16_t port);
    void listen(const std::function<void(int, const std::chrono::system_clock::time_point&, uint32_t)>& dataCallback);
    void leave(int sock, const std::string & addr);

    const std::string& getAddress(int sock);

private:
    bool mJoined;
    std::string mAddr;

    fd_set mReadSet;
    std::map<int, std::string> mSockets;
};
