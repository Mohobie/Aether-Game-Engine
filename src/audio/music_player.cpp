#include "music_player.h"
#include <iostream>
namespace aether {
MusicPlayer::MusicPlayer() {}
bool MusicPlayer::loadPlaylist(const std::vector<std::string>& tracks) {
    this->tracks = tracks;
    std::cout << "[Music] Loaded " << tracks.size() << " tracks" << std::endl;
    return true;
}
void MusicPlayer::play() { playing = true; }
void MusicPlayer::pause() { playing = false; }
void MusicPlayer::stop() { playing = false; currentTrack = 0; }
void MusicPlayer::next() { if (!tracks.empty()) currentTrack = (currentTrack + 1) % tracks.size(); }
void MusicPlayer::previous() { if (!tracks.empty()) currentTrack = (currentTrack + tracks.size() - 1) % tracks.size(); }
void MusicPlayer::setVolume(float volume) { this->volume = volume; }
bool MusicPlayer::isPlaying() const { return playing; }
} // namespace aether
