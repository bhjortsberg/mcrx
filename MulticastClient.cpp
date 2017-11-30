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
#include "MulticastClient.h"

std::pair<float, char> quantify(uint32_t bytes);

MulticastClient::MulticastClient() :
mJoined(false)
{
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

}

MulticastClient::MulticastClient(const std::string &addr) :
        mJoined(false)
{
    mAddr = addr;
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    join(addr);
}

MulticastClient::~MulticastClient()
{
    if (mJoined)
    {
        // Leave group
        leave(mAddr);
    }
    close(sock_fd);
}

void MulticastClient::join(const std::string & addr)
{
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(addr.c_str());
    mreq.imr_interface.s_addr = INADDR_ANY;
    if (!setsockopt(sock_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)))
    {
        mJoined = true;
        std::cout << "Joined " << addr << std::endl;
    }
    // TODO: Handle error
}

void MulticastClient::leave(const std::string & addr)
{
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(addr.c_str());
    mreq.imr_interface.s_addr = INADDR_ANY;
    if (!setsockopt(sock_fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq)))
    {
        mJoined = false;
        std::cout << "Leaving\n";
    }
}

void MulticastClient::listen(uint16_t port)
{
    struct sockaddr_in addr;
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock_fd, (struct sockaddr*)&addr, sizeof(addr)))
    {
        std::string error = "Bind error on port: " + std::to_string(port);
        perror(error.c_str());
        return;
    }

    char buffer[1024];
    socklen_t len;
    ssize_t bytes;

    std::cout << "Listening on port " << port << "..." << std::flush;
    uint32_t total = 0;
    while (true)
    {
        bytes = recvfrom(sock_fd, buffer, sizeof(buffer), 0, (struct sockaddr*)&addr, &len);
        if (bytes < 0)
        {
            perror("recvfrom");
            return;
        }
        total += bytes;
        auto [t, s] = quantify(total);
        std::cout << "\rReceived " << std::setw(10) << std::setprecision(5) << t << " " << s << "Bytes" << std::flush;
    }
}

std::pair<float, char> quantify(uint32_t bytes)
{
    float retBytes = static_cast<float>(bytes)/1024;
    std::pair<float, char> ret {bytes, ' '};
    if (retBytes > 1)
    {
        ret = std::make_pair(retBytes, 'K');
    }

    retBytes /= 1024;
    if (retBytes > 1)
    {
        ret = std::make_pair(retBytes, 'M');
    }
    retBytes /= 1024;
    if (retBytes > 1)
    {
        ret = std::make_pair(retBytes, 'G');
    }

    return ret;

}
