//
// Created by Björn Hjortsberg on 18/11/17.
//

#include <iostream>
#include <functional>
#include "MulticastClient.h"

#include <arpa/inet.h>
#include <iomanip>

std::vector<std::pair<std::string, uint16_t>> handleArguments(int argc, char **pString);
std::map<std::string, uint32_t> joinAll(const std::vector<std::pair<std::string, uint16_t>>& addresses,
                                        MulticastClient& client);
void usage(const std::string& appName);
std::pair<float, char> unitize(uint64_t bytes);


class InputError : public std::runtime_error
{
public:
    InputError(const std::string& errorStr):
            std::runtime_error(errorStr)
    {}
};


int main(int argc, char ** argv)
{
    MulticastClient mcClient;

    try
    {
        auto addresses = handleArguments(argc, argv);
        auto addressIndexMap = joinAll(addresses, mcClient);

        for (const auto& address : addresses)
        {
            std::cout << address.first << ":" << address.second << std::setw(20)  << "...listening" << std::endl;
        }

        std::map<int, uint64_t> totalBytes;
        auto printout = [&mcClient, &totalBytes, &addressIndexMap](int sock,
                                                                   const TimePoint& time,
                                                                   uint32_t bytes) {
            totalBytes[sock] += bytes;
            auto [t, s] = unitize(totalBytes[sock]);

            const std::string addrStr = mcClient.getAddress(sock);
            uint32_t cursorPos = addressIndexMap.size() - addressIndexMap.at(addrStr);

            std::cout << "\x1b[" + std::to_string(cursorPos) + "A";
            std::cout << "\r" << addrStr << std::setw(20) << std::setprecision(5)
                      << t << " " << s << "Bytes" << std::flush;
            std::cout << "\x1b[" + std::to_string(cursorPos) + "B";
        };

        mcClient.listen(printout);
    }
    catch (const InputError &e)
    {
        std::cout << "Error: " << e.what() << "\n";
        usage(argv[0]);
    }
    catch (const std::runtime_error& e)
    {
        std::cout << "Error: " << e.what() << "\n";
    }

    return 0;
}

std::vector<std::pair<std::string, uint16_t>> handleArguments(int argc, char **pString)
{
    std::vector<std::pair<std::string, uint16_t>> addressList;
    if (argc != 2)
    {
        throw InputError("Wrong number of arguments");
    }

    std::string addr;
    std::string mcNetAddrStr(pString[1]); // on the form 224.1.1.1:1234:1235
    bool addrFound = false;
    size_t delimPos = 0;
    size_t oldPos = 0;
    bool parsingDone = false;

    while (not parsingDone)
    {
        oldPos = delimPos;
        delimPos = mcNetAddrStr.find(":", delimPos + 1);
        if (delimPos == std::string::npos)
        {
            delimPos = mcNetAddrStr.size();
            parsingDone = true;
        }
        if (not addrFound)
        {
            addr = mcNetAddrStr.substr(0, delimPos);
            if (inet_addr(addr.c_str()) == INADDR_NONE) {
                throw InputError("Invalid address \"" + addr + "\"");
            }
            addrFound = true;
            continue;
        }

        auto portStr = mcNetAddrStr.substr(oldPos + 1, delimPos - oldPos - 1);
        char *endPtr;
        uint16_t port = std::strtol(portStr.c_str(), &endPtr, 10);
        if (*endPtr != '\0')
        {
            throw InputError("Not a valid port: " + portStr);
        }
        addressList.emplace_back(addr, port);
    }

    return addressList;
}


std::map<std::string, uint32_t> joinAll(const std::vector<std::pair<std::string, uint16_t>>& addresses,
                                        MulticastClient& client)
{
    std::map<std::string, uint32_t> addressIndexMap;
    uint32_t idx = 0;

    // Join all
    for (const auto& address : addresses)
    {
        client.join(address.first, address.second);
        addressIndexMap.emplace(address.first + ":" + std::to_string(address.second), idx);
        ++idx;
    }

    return addressIndexMap;
}

std::pair<float, char> unitize(uint64_t bytes)
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

void usage(const std::string& appName)
{
    std::string help;
    help  = "\tUsage:\n";
    help += "\t\t" + appName + " <multicast-ip>:<port1>[:<port2>:...<portN>]\n";
    std::cout << help << std::endl;
}
