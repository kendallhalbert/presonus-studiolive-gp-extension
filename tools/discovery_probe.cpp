/// Standalone UCNet UDP discovery probe (port 47809). For §11 hardware smoke prep.
/// Usage: discovery_probe.exe [timeoutMs]

#include "bridge/Logger.h"
#include "transport/DiscoveryListener.h"

#include <cstdlib>
#include <iostream>

int main(int argc, char *argv[])
{
    int timeoutMs = 5000;
    if (argc > 1)
    {
        timeoutMs = std::atoi(argv[1]);
        if (timeoutMs < 0)
        {
            timeoutMs = 0;
        }
    }

    presonus::studiolive::gpext::bridge::Logger logger;
    logger.setSink([](presonus::studiolive::gpext::bridge::LogLevel /*level*/,
                      std::string_view message) { std::cout << message << '\n'; });

    std::cout << "Listening on UDP 47809 for " << timeoutMs << " ms...\n";
    const auto devices = presonus::studiolive::gpext::transport::listenForDiscovery(
        std::chrono::milliseconds(timeoutMs), logger);

    std::cout << "Found " << devices.size() << " device(s)\n";
    for (std::size_t i = 0; i < devices.size(); ++i)
    {
        const auto &device = devices[i];
        std::cout << (i + 1) << ": " << device.name << " serial=" << device.serial
                  << " host=" << device.host << " tcp=" << device.tcpPort << '\n';
    }

    return devices.empty() ? 1 : 0;
}
