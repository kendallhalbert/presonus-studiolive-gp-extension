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

TEST(Phase0Sanity, VersionStringIsRelease)
{
    using presonus::studiolive::gpext::versionString;
    const auto v = versionString();
    EXPECT_FALSE(v.empty()) << "psl_Version() must return a non-empty string";
    EXPECT_EQ(v, "1.0.0") << "Release build should report semver; got: " << v;
}

} // namespace
