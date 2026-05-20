#include "bridge/RealGpHost.h"

#include "gigperformer/sdk/imports.h"

namespace presonus::studiolive::gpext::bridge
{

RealGpHost::RealGpHost(LibraryHandle handle) : handle_(handle)
{
}

void RealGpHost::consoleLog(const std::string &message)
{
    if (handle_ != nullptr && GP_ConsoleLog != nullptr)
    {
        GP_ConsoleLog(handle_, message.c_str());
    }
}

bool RealGpHost::widgetExists(const std::string &widgetName)
{
    if (handle_ == nullptr || GP_WidgetExists == nullptr)
    {
        return false;
    }
    return GP_WidgetExists(handle_, widgetName.c_str());
}

double RealGpHost::getWidgetValue(const std::string &widgetName)
{
    if (handle_ == nullptr || GP_GetWidgetValue == nullptr)
    {
        return 0.0;
    }
    return GP_GetWidgetValue(handle_, widgetName.c_str());
}

bool RealGpHost::setWidgetValue(const std::string &widgetName, double value)
{
    if (handle_ == nullptr || GP_SetWidgetValue == nullptr)
    {
        return false;
    }
    return GP_SetWidgetValue(handle_, widgetName.c_str(), value);
}

bool RealGpHost::listenForWidget(const std::string &widgetName, bool listen)
{
    if (handle_ == nullptr || GP_ListenForWidget == nullptr)
    {
        return false;
    }
    return GP_ListenForWidget(handle_, widgetName.c_str(), listen);
}

} // namespace presonus::studiolive::gpext::bridge
