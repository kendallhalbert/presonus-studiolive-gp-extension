#include "transport/MeterListener.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>

namespace presonus::studiolive::gpext::transport
{

namespace
{

bool ensureWsaStarted()
{
    static bool started = [] {
        WSADATA data{};
        return WSAStartup(MAKEWORD(2, 2), &data) == 0;
    }();
    return started;
}

} // namespace

MeterListener::MeterListener(bridge::Logger &logger) : logger_(logger) {}

MeterListener::~MeterListener()
{
    stop();
}

bool MeterListener::start(const std::uint16_t port)
{
    stop();

    if (!ensureWsaStarted())
    {
        logger_.warn("Meter listener: WSAStartup failed");
        return false;
    }

    const SOCKET socketHandle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socketHandle == INVALID_SOCKET)
    {
        logger_.warn("Meter listener: socket() failed");
        return false;
    }

    const BOOL reuse = TRUE;
    setsockopt(socketHandle, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<const char *>(&reuse), sizeof(reuse));

    sockaddr_in bindAddress{};
    bindAddress.sin_family = AF_INET;
    bindAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    bindAddress.sin_port = htons(port);
    if (bind(socketHandle, reinterpret_cast<sockaddr *>(&bindAddress), sizeof(bindAddress)) != 0)
    {
        logger_.warn("Meter listener: bind() failed on port " + std::to_string(port));
        closesocket(socketHandle);
        return false;
    }

    u_long nonBlocking = 1;
    ioctlsocket(socketHandle, FIONBIO, &nonBlocking);

    socketHandle_ = reinterpret_cast<void *>(socketHandle);
    port_ = port;
    running_ = true;
    logger_.info("Meter listener started on UDP port " + std::to_string(port));
    return true;
}

void MeterListener::stop()
{
    if (socketHandle_ == nullptr)
    {
        running_ = false;
        port_ = 0;
        return;
    }

    closesocket(reinterpret_cast<SOCKET>(socketHandle_));
    socketHandle_ = nullptr;
    if (running_)
    {
        logger_.info("Meter listener stopped");
    }
    running_ = false;
    port_ = 0;
}

bool MeterListener::poll(state::MeterCache &cache, FrameCallback onFrame)
{
    if (!running_ || socketHandle_ == nullptr)
    {
        return false;
    }

    const SOCKET socketHandle = reinterpret_cast<SOCKET>(socketHandle_);
    bool received = false;

    for (;;)
    {
        std::uint8_t buffer[2048];
        sockaddr_in sender{};
        int senderLength = sizeof(sender);
        const int bytes = recvfrom(socketHandle, reinterpret_cast<char *>(buffer),
                                   static_cast<int>(sizeof(buffer)), 0,
                                   reinterpret_cast<sockaddr *>(&sender), &senderLength);
        if (bytes <= 0)
        {
            break;
        }

        char hostBuffer[INET_ADDRSTRLEN] = {};
        inet_ntop(AF_INET, &sender.sin_addr, hostBuffer, sizeof(hostBuffer));

        const auto message = protocol::parseLevlPacket(
            std::span<const std::uint8_t>(buffer, static_cast<std::size_t>(bytes)));
        if (!message.has_value())
        {
            if (bytes >= 12)
            {
                const std::string tag(reinterpret_cast<const char *>(buffer + 12),
                                      std::min(4, bytes - 12));
                if (tag == "levl")
                {
                    logger_.warn("Meter levl frame rejected: " + std::to_string(bytes) +
                                 " bytes from " + hostBuffer);
                }
                else
                {
                    logger_.debug("Meter UDP ignored: " + std::to_string(bytes) +
                                    " bytes from " + hostBuffer + " code=" +
                                    std::string(1, static_cast<char>(buffer[6])) +
                                    std::string(1, static_cast<char>(buffer[7])) +
                                    " tag=" + tag);
                }
            }
            continue;
        }

        logger_.info("Meter levl frame from " + std::string(hostBuffer) + " (" +
                     std::to_string(bytes) + " bytes)");

        cache.apply(*message);
        if (onFrame)
        {
            onFrame(*message);
        }
        received = true;
    }

    return received;
}

} // namespace presonus::studiolive::gpext::transport
