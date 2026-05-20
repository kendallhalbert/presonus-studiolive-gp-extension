#include "protocol/ValueUtil.h"

#include <algorithm>
#include <cmath>

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

} // namespace presonus::studiolive::gpext::protocol
