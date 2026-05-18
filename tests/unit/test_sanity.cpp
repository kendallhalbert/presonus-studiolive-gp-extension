/// \file  test_sanity.cpp
/// \brief Smoke test that proves the GoogleTest + FetchContent + ctest
///        pipeline works.  Once Phase 1 lands, this file will likely be
///        replaced by the first real protocol test (UBJSON or KVTree).

#include <gtest/gtest.h>

#include "Version.h"

namespace
{

TEST(Phase0Sanity, GoogleTestBootstraps)
{
    EXPECT_EQ(1 + 1, 2);
}

TEST(Phase0Sanity, VersionStringIsNonEmptyAndPhase0)
{
    using presonus::studiolive::gpext::versionString;
    const auto v = versionString();
    EXPECT_FALSE(v.empty()) << "psl_Version() must return a non-empty string";
    EXPECT_NE(v.find("phase0"), std::string::npos)
        << "Phase 0 build should advertise itself; got: " << v;
}

} // namespace
