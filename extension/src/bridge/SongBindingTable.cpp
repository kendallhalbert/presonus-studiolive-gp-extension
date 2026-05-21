#include "bridge/SongBindingTable.h"

namespace presonus::studiolive::gpext::bridge
{

bool SongBindingTable::bindSong(int songIndex, std::string projectFile,
                                std::string sceneFile)
{
    if (songIndex < 0 || projectFile.empty() || sceneFile.empty())
    {
        return false;
    }
    bySong_[songIndex] = SongSceneBinding{
        .projectFile = std::move(projectFile),
        .sceneFile = std::move(sceneFile),
    };
    return true;
}

bool SongBindingTable::bindSongPart(int songIndex, int partIndex, std::string projectFile,
                                    std::string sceneFile)
{
    if (songIndex < 0 || partIndex < 0 || projectFile.empty() || sceneFile.empty())
    {
        return false;
    }
    bySongPart_[songIndex][partIndex] = SongSceneBinding{
        .projectFile = std::move(projectFile),
        .sceneFile = std::move(sceneFile),
    };
    return true;
}

bool SongBindingTable::unbindSong(int songIndex)
{
    bySong_.erase(songIndex);
    bySongPart_.erase(songIndex);
    return true;
}

void SongBindingTable::clear()
{
    bySong_.clear();
    bySongPart_.clear();
}

std::optional<SongSceneBinding> SongBindingTable::lookupSong(int songIndex) const
{
    const auto it = bySong_.find(songIndex);
    if (it == bySong_.end())
    {
        return std::nullopt;
    }
    return it->second;
}

std::optional<SongSceneBinding> SongBindingTable::lookupSongPart(int songIndex,
                                                                 int partIndex) const
{
    const auto songIt = bySongPart_.find(songIndex);
    if (songIt == bySongPart_.end())
    {
        return std::nullopt;
    }
    const auto partIt = songIt->second.find(partIndex);
    if (partIt == songIt->second.end())
    {
        return std::nullopt;
    }
    return partIt->second;
}

} // namespace presonus::studiolive::gpext::bridge
