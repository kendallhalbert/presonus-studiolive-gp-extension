#pragma once

#include <optional>
#include <string>
#include <unordered_map>

namespace presonus::studiolive::gpext::bridge
{

struct SongSceneBinding
{
    std::string projectFile;
    std::string sceneFile;
};

/// In-memory GP song index → mixer scene recall mapping (Phase 3).
class SongBindingTable
{
  public:
    bool bindSong(int songIndex, std::string projectFile, std::string sceneFile);
    bool bindSongPart(int songIndex, int partIndex, std::string projectFile,
                      std::string sceneFile);
    bool unbindSong(int songIndex);
    void clear();

    std::optional<SongSceneBinding> lookupSong(int songIndex) const;
    std::optional<SongSceneBinding> lookupSongPart(int songIndex, int partIndex) const;

  private:
    std::unordered_map<int, SongSceneBinding> bySong_;
    std::unordered_map<int, std::unordered_map<int, SongSceneBinding>> bySongPart_;
};

} // namespace presonus::studiolive::gpext::bridge
