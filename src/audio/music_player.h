#pragma once
#include <string>
#include <vector>
namespace aether {
class MusicPlayer {
public:
    MusicPlayer();
    bool loadPlaylist(const std::vector<std::string>& tracks);
    void play();
    void pause();
    void stop();
    void next();
    void previous();
    void setVolume(float volume);
    bool isPlaying() const;
private:
    std::vector<std::string> tracks;
    size_t currentTrack = 0;
    float volume = 1.0f;
    bool playing = false;
};
} // namespace aether
