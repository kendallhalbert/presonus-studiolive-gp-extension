#include <gtest/gtest.h>

#include <string>

#include "FixtureUtil.h"
#include "protocol/FdAssembler.h"
#include "protocol/FdParser.h"
#include "protocol/MessageProtocol.h"

namespace
{

TEST(FdAssembler, ProjectsListReassemblesJson)
{
    presonus::studiolive::gpext::protocol::FdAssembler assembler;
    std::optional<std::vector<std::uint8_t>> json;

    for (const char *name : {"000.bin", "002.bin", "001.bin"})
    {
        const auto frame = readFixtureBin((std::string("14-fd-projects-list/") + name).c_str());
        const auto wire = presonus::studiolive::gpext::protocol::analysePacket(frame);
        ASSERT_TRUE(wire.has_value());
        EXPECT_EQ(wire->messageCode, "FD");
        if (const auto chunk = assembler.addChunk(wire->payload))
        {
            json = *chunk;
            break;
        }
    }

    ASSERT_TRUE(json.has_value());
    const std::string text(reinterpret_cast<const char *>(json->data()), json->size());
    const auto files = presonus::studiolive::gpext::protocol::parseFdFileList(text);
    ASSERT_TRUE(files.has_value());
    ASSERT_GE(files->size(), 2u);
    EXPECT_EQ((*files)[0].name, "01.West End Girls.proj");
    EXPECT_EQ((*files)[1].name, "02.Rush.proj");
}

} // namespace
