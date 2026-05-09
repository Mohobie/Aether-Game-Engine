#include "audio_engine.h"
#include <iostream>

namespace vge {
bool AudioEngine::Initialize() {
    std::cout << "[Audio] Initialized (stub)" << std::endl;
    return true;
}

void AudioEngine::Shutdown() {
    std::cout << "[Audio] Shutdown" << std::endl;
}

void AudioEngine::PlaySound(const std::string& file) {
    std::cout << "[Audio] Playing: " << file << std::endl;
}

void AudioEngine::SetMasterVolume(float volume) {
    std::cout << "[Audio] Volume: " << volume << std::endl;
}
}