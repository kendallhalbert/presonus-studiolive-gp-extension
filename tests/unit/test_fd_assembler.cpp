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
    std::optional<presonus::studiolive::gpext::protocol::FdAssemblyResult> assembled;

    for (const char *name : {"000.bin", "002.bin", "001.bin"})
    {
        const auto frame = readFixtureBin((std::string("14-fd-projects-list/") + name).c_str());
        const auto wire = presonus::studiolive::gpext::protocol::analysePacket(frame);
        ASSERT_TRUE(wire.has_value());
        EXPECT_EQ(wire->messageCode, "FD");
        if (const auto chunk = assembler.addChunk(wire->payload))
        {
            assembled = *chunk;
            break;
        }
    }

    ASSERT_TRUE(assembled.has_value());
    const std::string text(reinterpret_cast<const char *>(assembled->json.data()),
                           assembled->json.size());
    const auto files = presonus::studiolive::gpext::protocol::parseFdFileList(text);
    ASSERT_TRUE(files.has_value());
    ASSERT_GE(files->size(), 2u);
    EXPECT_EQ((*files)[0].name, "01.West End Girls.proj");
    EXPECT_EQ((*files)[1].name, "02.Rush.proj");
}

TEST(FdParser, SceneListFiltersCnfgAndKeepsScn)
{
    // From session capture: List presets/proj/01.West End Girls.proj (2026-05-18)
    constexpr const char *kSceneListJson =
        R"({"files": [{"name": "West End Girls.cnfg", "title": "West End Girls.cnfg"}, )"
        R"({"name": "01.Live Performance.scn", "title": "Live Performance"}, )"
        R"({"name": "02._ Empty Location _.scn", "title": "* Empty Location *"}]})";

    const auto files = presonus::studiolive::gpext::protocol::parseFdFileList(kSceneListJson);
    ASSERT_TRUE(files.has_value());
    EXPECT_EQ(files->size(), 3u);

    const auto scenes = presonus::studiolive::gpext::protocol::filterFdSceneFiles(*files);
    ASSERT_EQ(scenes.size(), 2u);
    EXPECT_EQ(scenes[0].name, "01.Live Performance.scn");
    EXPECT_EQ(scenes[0].title, "Live Performance");
    EXPECT_EQ(scenes[1].name, "02._ Empty Location _.scn");
    EXPECT_FALSE(presonus::studiolive::gpext::protocol::isFdSceneFile("West End Girls.cnfg"));
    EXPECT_TRUE(presonus::studiolive::gpext::protocol::isFdSceneFile("01.Live Performance.scn"));
}

TEST(FdParser, ChannelPresetListFiltersLockAndCnfg)
{
    const std::vector<presonus::studiolive::gpext::protocol::FdFileEntry> entries = {
        {.name = "01.Vocal.ch", .title = "Vocal"},
        {.name = "52.Siku.Wind.channel", .title = "Siku Wind"},
        {.name = "West End Girls.cnfg", .title = "West End Girls.cnfg"},
        {.name = "02._ Empty Location _.ch", .title = "* Empty Location *"},
        {.name = "show.lock", .title = "show.lock"},
        {.name = "metadata-only", .title = "Not a preset file"},
    };

    const auto presets =
        presonus::studiolive::gpext::protocol::filterFdChannelPresetFiles(entries);
    ASSERT_EQ(presets.size(), 2u);
    EXPECT_EQ(presets[0].name, "01.Vocal.ch");
    EXPECT_EQ(presets[1].name, "52.Siku.Wind.channel");
    EXPECT_TRUE(presonus::studiolive::gpext::protocol::isFdChannelPresetFile("01.Vocal.ch"));
    EXPECT_TRUE(presonus::studiolive::gpext::protocol::isFdChannelPresetFile("52.Siku.Wind.channel"));
}

} // namespace
