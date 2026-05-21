#include "protocol/ValueUtil.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <thread>

namespace presonus::studiolive::gpext::protocol
{

namespace
{

double clamp(double value, double lo, double hi)
{
    return std::max(lo, std::min(value, hi));
}

} // namespace

float linearPercentToVolumeScalar(double linearPercent)
{
    return static_cast<float>(clamp(linearPercent, 0.0, 100.0) / 100.0);
}

double volumeScalarToLinearPercent(double scalar)
{
    if (scalar <= 1.0)
    {
        return clamp(scalar, 0.0, 1.0) * 100.0;
    }
    return clamp(scalar, 0.0, 100.0);
}

float panPercentToScalar(double panPercent)
{
    return static_cast<float>(clamp(panPercent, 0.0, 100.0) / 100.0);
}

double panScalarToPercent(double scalar)
{
    if (scalar <= 1.0)
    {
        return clamp(scalar, 0.0, 1.0) * 100.0;
    }
    return clamp(scalar, 0.0, 100.0);
}

double dbToLinearPercent(double db)
{
    constexpr double kMinDb = -84.0;
    constexpr double kMaxDb = 10.0;
    db = clamp(db, kMinDb, kMaxDb);
    if (db <= kMinDb)
    {
        return 0.0;
    }
    if (db >= kMaxDb)
    {
        return 100.0;
    }

    const double linear = 72.5204177782 + 2.473473992 * db + 0.026567557 * db * db +
                        0.0000880866 * db * db * db;
    return clamp(linear, 0.0, 100.0);
}

double linearPercentToDb(double linearPercent)
{
    constexpr double kMinDb = -84.0;
    constexpr double kMaxDb = 10.0;
    linearPercent = clamp(linearPercent, 0.0, 100.0);
    if (linearPercent <= 0.0)
    {
        return kMinDb;
    }
    if (linearPercent >= 100.0)
    {
        return kMaxDb;
    }

    double lo = kMinDb;
    double hi = kMaxDb;
    for (int i = 0; i < 40; ++i)
    {
        const double mid = (lo + hi) * 0.5;
        if (dbToLinearPercent(mid) < linearPercent)
        {
            lo = mid;
        }
        else
        {
            hi = mid;
        }
    }
    return (lo + hi) * 0.5;
}

void transitionValue(const double from,
                     const double to,
                     const int durationMs,
                     const std::function<void(double)> &onStep,
                     const std::function<void()> &onDone)
{
    if (durationMs <= 0 || from == to)
    {
        onStep(to);
        if (onDone)
        {
            onDone();
        }
        return;
    }

    constexpr int kMinIntervalMs = 10;
    const int intervalMs = std::max(durationMs / 100, kMinIntervalMs);
    const double step = clamp(static_cast<double>(intervalMs) / durationMs, 0.0, 1.0);

    constexpr double kPi = 3.14159265358979323846;
    const auto curve = [kPi](const double position) {
        return -(std::cos(kPi * position) - 1.0) / 2.0;
    };

    double progress = 0.0;
    for (;;)
    {
        onStep(from + (to - from) * curve(progress));
        if (progress >= 1.0)
        {
            break;
        }
        progress = std::min(progress + step, 1.0);
        std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
    }

    if (onDone)
    {
        onDone();
    }
}

} // namespace presonus::studiolive::gpext::protocol
