//
// Created by Björn Hjortsberg on 18/11/17.
//

#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <functional>
#include <map>

using TimePoint = std::chrono::system_clock::time_point;

class MulticastClient
{
public:
    MulticastClient();
    ~MulticastClient();

    void join(const std::string& addr, uint16_t port);
    void listen(const std::function<void(int, const TimePoint&, uint32_t)>& dataCallback);
    void leave(int sock, const std::string & addr);
    void leaveAll();
    void stop();

    const std::string& getAddress(int sock);

private:
    fd_set mReadSet;
    std::map<int, std::string> mSockets;
    bool mRun = true;
};
