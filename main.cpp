//
// Created by Björn Hjortsberg on 18/11/17.
//

#include <iostream>
#include <functional>
#include "MulticastClient.h"


#include <arpa/inet.h>

std::pair<std::string, uint16_t> handleArguments(int argc, char **pString);
void usage(const std::string& appName);

int main(int argc, char ** argv)
{
    MulticastClient mcClient;

    try
    {
        auto [addr, port] = handleArguments(argc, argv);
        mcClient.join(addr);
        mcClient.listen(port);
    }
    catch (const std::runtime_error &e)
    {
        std::cout << "Error: " << e.what() << "\n";
        usage(argv[0]);
    }

    return 0;
}

void usage(const std::string& appName)
{
    std::string help;
    help  = "\tUsage:\n";
    help += "\t\t" + appName + " <multicast-ip> <port>\n";
    std::cout << help << std::endl;
}

std::pair<std::string, uint16_t> handleArguments(int argc, char **pString) {
    if (argc != 3)
    {
        throw std::runtime_error("Wrong number of arguments");
    }
    std::string addr(pString[1]);
    char* endPtr;
    uint16_t port = std::strtol(pString[2], &endPtr, 10);
    if (*endPtr != '\0')
    {
        throw std::runtime_error("Not a valid port");
    }

    if (inet_addr(addr.c_str()) == INADDR_NONE) {
        throw std::runtime_error("Invalid address \"" + addr + "\"");
    }
    return {addr, port};
}
