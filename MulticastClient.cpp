//
// Created by Björn Hjortsberg on 18/11/17.
//

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <zconf.h>
#include <arpa/inet.h>
#include <iostream>
#include <iomanip>
#include <asm/ioctls.h>
#include <stropts.h>
#include <cstring>
#include "MulticastClient.h"


MulticastClient::MulticastClient() :
mJoined(false)
{
    FD_ZERO(&mReadSet);
}


MulticastClient::~MulticastClient()
{
    if (mJoined)
    {
        // Leave group
        for (const auto& a : mSockets) {
            leave(a.first, a.second);
        }
    }
}

void MulticastClient::leave(int sock, const std::string & addr)
{
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(addr.c_str());
    mreq.imr_interface.s_addr = INADDR_ANY;
    if (!setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq)))
    {
        mJoined = false;
        std::cout << "Leaving\n";
    }
}


void MulticastClient::join(const std::string &addr, uint16_t port)
{
    int sock = ::socket(AF_INET, SOCK_DGRAM, 0);
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(addr.c_str());
    mreq.imr_interface.s_addr = INADDR_ANY;
    if (!setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP | SO_REUSEADDR, &mreq, sizeof(mreq)))
    {
        mJoined = true;
        std::cout << "Joined " << addr << std::endl;
    }

    // Set non blocking socket
    uint32_t nonblock = 1;
    ioctl(sock, FIONBIO, &nonblock);

    // Bind address to socket
    struct sockaddr_in ipAddr;
    ipAddr.sin_port = htons(port);
    ipAddr.sin_family = AF_INET;
    ipAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr*)&ipAddr, sizeof(ipAddr)))
    {
        std::string error = "Bind error on port: " + std::to_string(port);
        error += " (" + std::string(strerror(errno)) + ")";
        throw std::runtime_error(error);
    }

    FD_SET(sock, &mReadSet);

    mSockets.emplace(sock, addr + ":" + std::to_string(port));
}

void MulticastClient::listen(const std::function<void(int, const std::chrono::system_clock::time_point&, uint32_t)>& dataCallback)
{
    struct sockaddr_in addr;
    char buffer[3*1024];
    socklen_t len;
    ssize_t bytes;

    fd_set testset;
    int result;

    while (true)
    {
        testset = mReadSet;
        result = select(FD_SETSIZE, &testset, NULL, NULL, NULL);
        for (const auto& [sock, _] : mSockets)
        {
            if (FD_ISSET(sock, &testset))
            {

                uint32_t total = 0;
                while ((bytes = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *) &addr, &len)) > 0)
                {
                    total += bytes;
                }

                if (bytes < 0 && errno != EAGAIN)
                {
                    // Fail if we stopped reading due to something
                    // else than there is no data
                    perror("recvfrom");
                    return;
                }

                dataCallback(sock, std::chrono::system_clock::now(), total);
            }
        }
    }

}

const std::string& MulticastClient::getAddress(int sock) {
    return mSockets.at(sock);
}
