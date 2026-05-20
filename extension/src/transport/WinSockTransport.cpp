#include "transport/WinSockTransport.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>

#include <cstring>

namespace presonus::studiolive::gpext::transport
{

namespace
{

SOCKET toSocket(void *handle) { return reinterpret_cast<SOCKET>(handle); }

void *fromSocket(SOCKET socket) { return reinterpret_cast<void *>(socket); }

} // namespace

WinSockTransport::WinSockTransport() = default;

WinSockTransport::~WinSockTransport()
{
    close();
}

bool WinSockTransport::ensureWsaStarted()
{
    static bool started = [] {
        WSADATA data{};
        return WSAStartup(MAKEWORD(2, 2), &data) == 0;
    }();
    return started;
}

bool WinSockTransport::connect(const std::string &host, std::uint16_t port)
{
    if (!ensureWsaStarted())
    {
        return false;
    }

    close();

    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    addrinfo *result = nullptr;
    const std::string portString = std::to_string(port);
    if (getaddrinfo(host.c_str(), portString.c_str(), &hints, &result) != 0)
    {
        return false;
    }

    SOCKET socketHandle = INVALID_SOCKET;
    for (addrinfo *cursor = result; cursor != nullptr; cursor = cursor->ai_next)
    {
        socketHandle = socket(cursor->ai_family, cursor->ai_socktype, cursor->ai_protocol);
        if (socketHandle == INVALID_SOCKET)
        {
            continue;
        }
        if (::connect(socketHandle, cursor->ai_addr, static_cast<int>(cursor->ai_addrlen)) == 0)
        {
            break;
        }
        closesocket(socketHandle);
        socketHandle = INVALID_SOCKET;
    }

    freeaddrinfo(result);

    if (socketHandle == INVALID_SOCKET)
    {
        return false;
    }

    u_long nonBlocking = 1;
    ioctlsocket(socketHandle, FIONBIO, &nonBlocking);
    socket_ = fromSocket(socketHandle);
    return true;
}

bool WinSockTransport::write(std::span<const std::uint8_t> bytes)
{
    if (!isConnected())
    {
        return false;
    }

    std::size_t sent = 0;
    while (sent < bytes.size())
    {
        const int result = ::send(toSocket(socket_), reinterpret_cast<const char *>(bytes.data() + sent),
                                  static_cast<int>(bytes.size() - sent), 0);
        if (result <= 0)
        {
            return false;
        }
        sent += static_cast<std::size_t>(result);
    }
    return true;
}

void WinSockTransport::close()
{
    if (socket_ != nullptr)
    {
        closesocket(toSocket(socket_));
        socket_ = nullptr;
    }
}

bool WinSockTransport::isConnected() const
{
    return socket_ != nullptr;
}

void WinSockTransport::pollInbound()
{
    pumpReceive();
}

void WinSockTransport::pumpReceive()
{
    if (!isConnected())
    {
        return;
    }

    std::uint8_t buffer[4096];
    while (true)
    {
        const int received =
            recv(toSocket(socket_), reinterpret_cast<char *>(buffer), sizeof(buffer), 0);
        if (received > 0)
        {
            emitReceive(std::span<const std::uint8_t>(buffer, static_cast<std::size_t>(received)));
            continue;
        }
        if (received == 0 || WSAGetLastError() != WSAEWOULDBLOCK)
        {
            close();
        }
        break;
    }
}

} // namespace presonus::studiolive::gpext::transport
