#include <gtest/gtest.h>

#include <vector>

#include "protocol/ValueUtil.h"

namespace
{

TEST(ValueUtil, LinearVolumeRoundTrip)
{
    EXPECT_FLOAT_EQ(presonus::studiolive::gpext::protocol::linearPercentToVolumeScalar(72.0),
                    0.72F);
    EXPECT_DOUBLE_EQ(
        presonus::studiolive::gpext::protocol::volumeScalarToLinearPercent(0.72), 72.0);
}

TEST(ValueUtil, PanPercentRoundTrip)
{
    EXPECT_FLOAT_EQ(presonus::studiolive::gpext::protocol::panPercentToScalar(50.0), 0.5F);
    EXPECT_DOUBLE_EQ(presonus::studiolive::gpext::protocol::panScalarToPercent(0.5), 50.0);
}

TEST(ValueUtil, DbCurveMatchesJsReferencePoints)
{
    EXPECT_DOUBLE_EQ(presonus::studiolive::gpext::protocol::dbToLinearPercent(-84.0), 0.0);
    EXPECT_DOUBLE_EQ(presonus::studiolive::gpext::protocol::dbToLinearPercent(10.0), 100.0);
    const double mid = presonus::studiolive::gpext::protocol::dbToLinearPercent(-20.0);
    EXPECT_GT(mid, 25.0);
    EXPECT_LT(mid, 45.0);
    EXPECT_NEAR(presonus::studiolive::gpext::protocol::linearPercentToDb(mid), -20.0, 0.5);
}

TEST(ValueUtil, TransitionValueInstantWhenZeroDuration)
{
    double last = -1.0;
    presonus::studiolive::gpext::protocol::transitionValue(
        10.0, 90.0, 0, [&](const double value) { last = value; });
    EXPECT_DOUBLE_EQ(last, 90.0);
}

TEST(ValueUtil, TransitionValueStepsToTarget)
{
    std::vector<double> steps;
    presonus::studiolive::gpext::protocol::transitionValue(
        0.0, 100.0, 200, [&](const double value) { steps.push_back(value); });
    EXPECT_GE(steps.size(), 10U);
    EXPECT_DOUBLE_EQ(steps.front(), 0.0);
    EXPECT_NEAR(steps.back(), 100.0, 0.01);
}

} // namespace
