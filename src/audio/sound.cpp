#include "sound.h"
#include <iostream>
namespace aether {
Sound::Sound() {}
bool Sound::loadFromFile(const std::string& path) {
    std::cout << "[Sound] Loaded: " << path << std::endl;
    return true;
}
void Sound::play() { playing = true; }
void Sound::stop() { playing = false; }
void Sound::setVolume(float volume) { this->volume = volume; }
void Sound::setLooping(bool loop) { looping = loop; }
bool Sound::isPlaying() const { return playing; }
} // namespace aether
