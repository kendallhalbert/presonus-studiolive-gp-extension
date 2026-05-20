#include "mixer/MixerService.h"

#include "protocol/ConnectionHandshake.h"
#include "protocol/FileRequest.h"
#include "protocol/JmPacket.h"
#include "protocol/MixerConnection.h"
#include "protocol/PvEncoder.h"
#include "transport/WinSockTransport.h"

#include <chrono>
#include <utility>

namespace presonus::studiolive::gpext::mixer
{

MixerService::MixerService(bridge::Logger &logger) : logger_(logger) {}

MixerService::~MixerService()
{
    if (running_.load())
    {
        enqueue([this]() {
            if (connection_)
            {
                connection_->close();
                connection_.reset();
            }
            connected_ = false;
        });
        running_ = false;
        if (ioThread_.joinable())
        {
            ioThread_.join();
        }
    }
}

void MixerService::ensureThread()
{
    if (running_.exchange(true))
    {
        return;
    }

    ioThread_ = std::thread([this]() { ioLoop(); });
}

void MixerService::stopThread()
{
    if (!running_.exchange(false))
    {
        return;
    }

    if (ioThread_.joinable())
    {
        ioThread_.join();
    }
}

void MixerService::enqueue(IoTask task)
{
    {
        std::lock_guard lock(mutex_);
        tasks_.push_back(std::move(task));
    }
}

void MixerService::ioLoop()
{
    while (running_.load())
    {
        std::vector<IoTask> batch;
        {
            std::lock_guard lock(mutex_);
            batch.swap(tasks_);
        }

        for (auto &task : batch)
        {
            if (task)
            {
                task();
            }
        }

        if (connection_)
        {
            connection_->poll();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

std::uint16_t MixerService::allocateRequestId()
{
    return nextRequestId_.fetch_add(1);
}

bool MixerService::connect(const std::string &host, std::uint16_t port)
{
    ensureThread();

    std::atomic<bool> ok{false};
    std::atomic<bool> done{false};
    enqueue([this, host, port, &ok, &done]() {
        auto transport = std::make_unique<transport::WinSockTransport>();
        connection_ = std::make_unique<protocol::MixerConnection>(std::move(transport));
        connected_ = false;

        if (!connection_->connect(host, port))
        {
            logger_.warn("Mixer TCP connect failed: " + host);
            connection_.reset();
            ok.store(false);
            done.store(true);
            return;
        }

        protocol::ConnectionHandshake handshake;
        connection_->setSessionPacketCallback(
            [&handshake](const protocol::SessionPacket &packet) {
                handshake.onSessionPacket(packet);
            });
        connection_->setJsonMessageCallback([&handshake](std::string_view json) {
            handshake.onJmJson(json);
        });
        connection_->setKeepAliveEnabled(false);

        if (!connection_->sendRaw(protocol::createSubscribePacket()))
        {
            logger_.warn("Mixer subscribe send failed: " + host);
            connection_->close();
            connection_.reset();
            ok.store(false);
            done.store(true);
            return;
        }

        const auto deadline =
            std::chrono::steady_clock::now() + std::chrono::seconds(30);
        while (!handshake.progress().complete() &&
               std::chrono::steady_clock::now() < deadline)
        {
            connection_->poll();
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }

        if (!handshake.progress().complete())
        {
            logger_.warn("Mixer handshake timed out: " + host);
            connection_->close();
            connection_.reset();
            ok.store(false);
            done.store(true);
            return;
        }

        connection_->setKeepAliveEnabled(true);
        connected_ = true;
        logger_.info("Mixer connected to " + host);
        ok.store(true);
        done.store(true);
    });

    for (int i = 0; i < 3500 && !done.load(); ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return ok.load();
}

void MixerService::disconnect()
{
    enqueue([this]() {
        if (connection_)
        {
            connection_->close();
            connection_.reset();
        }
        connected_ = false;
        logger_.info("Mixer disconnected");
    });
}

bool MixerService::isConnected() const
{
    return connected_.load();
}

bool MixerService::setLineMute(int channel, bool muted)
{
    if (channel < 1 || !isConnected())
    {
        return false;
    }

    const auto key = protocol::lineChannelMuteKey(channel);
    const auto packet = protocol::createPvBoolPacket(key, muted);
    enqueue([this, packet = std::move(packet)]() {
        if (connection_)
        {
            connection_->sendRaw(packet);
        }
    });
    return true;
}

bool MixerService::requestFileList(const std::string &path)
{
    if (!isConnected())
    {
        return false;
    }

    const std::uint16_t requestId = allocateRequestId();
    const auto packet = protocol::createFileListRequestPacket(requestId, path);
    enqueue([this, packet = std::move(packet)]() {
        if (connection_)
        {
            connection_->sendRaw(packet);
        }
    });
    return true;
}

} // namespace presonus::studiolive::gpext::mixer
