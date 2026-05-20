#pragma once

#include <filesystem>
#include <fstream>
#include <vector>

inline std::vector<std::uint8_t> readFixtureBin(const char *relativePath)
{
    const std::filesystem::path path =
        std::filesystem::path(PSL_FIXTURE_ROOT) / relativePath;
    std::ifstream in(path, std::ios::binary);
    return std::vector<std::uint8_t>(std::istreambuf_iterator<char>(in), {});
}
