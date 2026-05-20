#pragma once

#include "bridge/GpHost.h"

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace presonus::studiolive::gpext::bridge
{

/// In-memory GpHost for unit tests. Records console logs and widget writes.
class MockGpHost final : public GpHost
{
  public:
    void consoleLog(const std::string &message) override
    {
        consoleLogs_.push_back(message);
    }

    bool widgetExists(const std::string &widgetName) override
    {
        return widgets_.count(widgetName) > 0;
    }

    double getWidgetValue(const std::string &widgetName) override
    {
        const auto it = widgets_.find(widgetName);
        return it != widgets_.end() ? it->second : 0.0;
    }

    bool setWidgetValue(const std::string &widgetName, double value) override
    {
        widgets_[widgetName] = value;
        widgetWrites_.emplace_back(widgetName, value);
        return true;
    }

    bool listenForWidget(const std::string &widgetName, bool listen) override
    {
        listenCalls_.emplace_back(widgetName, listen);
        return true;
    }

    const std::vector<std::string> &consoleLogs() const { return consoleLogs_; }

    const std::vector<std::pair<std::string, double>> &widgetWrites() const
    {
        return widgetWrites_;
    }

    void setWidget(const std::string &name, double value) { widgets_[name] = value; }

  private:
    std::vector<std::string> consoleLogs_;
    std::vector<std::pair<std::string, double>> widgetWrites_;
    std::vector<std::pair<std::string, bool>> listenCalls_;
    std::unordered_map<std::string, double> widgets_;
};

} // namespace presonus::studiolive::gpext::bridge
