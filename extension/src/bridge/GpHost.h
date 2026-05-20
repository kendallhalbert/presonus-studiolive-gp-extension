#pragma once

#include <string>

namespace presonus::studiolive::gpext::bridge
{

/// Testable facade over GP_* imports. All production GP calls from the
/// extension should go through this interface (not raw GP_* globals).
class GpHost
{
  public:
    virtual ~GpHost() = default;

    virtual void consoleLog(const std::string &message) = 0;

    virtual bool widgetExists(const std::string &widgetName) = 0;
    virtual double getWidgetValue(const std::string &widgetName) = 0;
    virtual bool setWidgetValue(const std::string &widgetName, double value) = 0;
    virtual bool listenForWidget(const std::string &widgetName, bool listen) = 0;
};

} // namespace presonus::studiolive::gpext::bridge
