#pragma once
#include <string>
namespace aether {
class Sound {
public:
    Sound();
    bool loadFromFile(const std::string& path);
    void play();
    void stop();
    void setVolume(float volume);
    void setLooping(bool loop);
    bool isPlaying() const;
private:
    float volume = 1.0f;
    bool looping = false;
    bool playing = false;
};
} // namespace aether
