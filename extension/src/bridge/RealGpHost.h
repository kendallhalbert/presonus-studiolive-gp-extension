#pragma once

#include "bridge/GpHost.h"

#include "gigperformer/sdk/types.h"

namespace presonus::studiolive::gpext::bridge
{

class RealGpHost final : public GpHost
{
  public:
    explicit RealGpHost(LibraryHandle handle);

    void consoleLog(const std::string &message) override;

    bool widgetExists(const std::string &widgetName) override;
    double getWidgetValue(const std::string &widgetName) override;
    bool setWidgetValue(const std::string &widgetName, double value) override;
    bool listenForWidget(const std::string &widgetName, bool listen) override;

  private:
    LibraryHandle handle_;
};

} // namespace presonus::studiolive::gpext::bridge
