#include <gtest/gtest.h>

#include <array>
#include <string>

#include "protocol/Ubjson.h"

namespace
{

TEST(Ubjson, ParsesSynchronizeIdField)
{
    const std::string raw = "{i\x02idSi\x0bSynchronize}";
    const auto root = presonus::studiolive::gpext::protocol::parseUbjsonObject(
        std::span<const std::uint8_t>(reinterpret_cast<const std::uint8_t *>(raw.data()),
                                      raw.size()));
    ASSERT_TRUE(root.has_value());
    const auto it = root->find("id");
    ASSERT_NE(it, root->end());
    ASSERT_TRUE(it->second.isString());
    EXPECT_EQ(*it->second.asString(), "Synchronize");
}

} // namespace
