#include "transport/DiscoveryListener.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>

#include <unordered_map>

namespace presonus::studiolive::gpext::transport
{

namespace
{

constexpr std::uint16_t kDiscoveryPort = 47809;

bool ensureWsaStarted()
{
    static bool started = [] {
        WSADATA data{};
        return WSAStartup(MAKEWORD(2, 2), &data) == 0;
    }();
    return started;
}

} // namespace

std::vector<protocol::DiscoveredMixer>
listenForDiscovery(const std::chrono::milliseconds timeout, bridge::Logger &logger)
{
    std::vector<protocol::DiscoveredMixer> devices;
    if (!ensureWsaStarted())
    {
        logger.warn("Discovery: WSAStartup failed");
        return devices;
    }

    const SOCKET socketHandle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socketHandle == INVALID_SOCKET)
    {
        logger.warn("Discovery: socket() failed");
        return devices;
    }

    const BOOL reuse = TRUE;
    setsockopt(socketHandle, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<const char *>(&reuse), sizeof(reuse));

    sockaddr_in bindAddress{};
    bindAddress.sin_family = AF_INET;
    bindAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    bindAddress.sin_port = htons(kDiscoveryPort);
    if (bind(socketHandle, reinterpret_cast<sockaddr *>(&bindAddress), sizeof(bindAddress)) != 0)
    {
        logger.warn("Discovery: bind() on port 47809 failed");
        closesocket(socketHandle);
        return devices;
    }

    const BOOL broadcast = TRUE;
    setsockopt(socketHandle, SOL_SOCKET, SO_BROADCAST,
               reinterpret_cast<const char *>(&broadcast), sizeof(broadcast));

    const auto deadline = std::chrono::steady_clock::now() + timeout;
    std::unordered_map<std::string, protocol::DiscoveredMixer> bySerial;

    while (std::chrono::steady_clock::now() < deadline)
    {
        const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
            deadline - std::chrono::steady_clock::now());
        if (remaining.count() <= 0)
        {
            break;
        }

        timeval tv{};
        tv.tv_sec = static_cast<long>(remaining.count() / 1000);
        tv.tv_usec = static_cast<long>((remaining.count() % 1000) * 1000);
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(socketHandle, &readSet);

        const int ready = select(0, &readSet, nullptr, nullptr, &tv);
        if (ready <= 0)
        {
            break;
        }

        std::uint8_t buffer[2048];
        sockaddr_in sender{};
        int senderLength = sizeof(sender);
        const int received = recvfrom(socketHandle, reinterpret_cast<char *>(buffer),
                                      static_cast<int>(sizeof(buffer)), 0,
                                      reinterpret_cast<sockaddr *>(&sender),
                                      &senderLength);
        if (received <= 0)
        {
            continue;
        }

        char hostBuffer[INET_ADDRSTRLEN] = {};
        inet_ntop(AF_INET, &sender.sin_addr, hostBuffer, sizeof(hostBuffer));

        const auto mixer = protocol::parseDiscoveryPacket(
            std::span<const std::uint8_t>(buffer, static_cast<std::size_t>(received)),
            hostBuffer);
        if (!mixer.has_value())
        {
            continue;
        }

        bySerial[mixer->serial] = *mixer;
        logger.info("Discovery: " + mixer->name + " serial=" + mixer->serial + " @ " +
                    mixer->host);
    }

    closesocket(socketHandle);

    devices.reserve(bySerial.size());
    for (const auto &entry : bySerial)
    {
        devices.push_back(entry.second);
    }
    return devices;
}

} // namespace presonus::studiolive::gpext::transport
