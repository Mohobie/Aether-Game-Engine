#include "audio/audio_engine.h"
#include "audio/sound_manager.h"
#include "core/logger.h"
#include <iostream>
#include <fstream>
#include <cstring>

// Simple audio engine using system audio (aplay/afplay)
// Can be extended with OpenAL or SDL2_mixer

namespace vge {

AudioEngine::AudioEngine() : initialized(false) {}

AudioEngine::~AudioEngine() {
    if (initialized) Shutdown();
}

bool AudioEngine::Initialize() {
    // Check if we can play audio
    std::cout << "[Audio] Checking audio capabilities..." << std::endl;
    
    // Try to detect available audio player
    int result = system("which aplay > /dev/null 2>&1");
    if (result == 0) {
        std::cout << "[Audio] Using aplay for audio playback" << std::endl;
    } else {
        std::cout << "[Audio] No audio player found (aplay not available)" << std::endl;
        std::cout << "[Audio] Audio will be disabled" << std::endl;
        return false;
    }
    
    initialized = true;
    std::cout << "[Audio] Audio engine initialized" << std::endl;
    return true;
}

void AudioEngine::Shutdown() {
    initialized = false;
    std::cout << "[Audio] Audio engine shutdown" << std::endl;
}

void AudioEngine::PlaySound(const std::string& file) {
    if (!initialized) return;
    
    std::string fullPath = "assets/sounds/" + file;
    
    // Check if file exists
    std::ifstream test(fullPath);
    if (!test.good()) {
        // Try without prefix
        test.close();
        test.open(file);
        if (!test.good()) {
            std::cerr << "[Audio] Sound file not found: " << fullPath << std::endl;
            return;
        }
        fullPath = file;
    }
    test.close();
    
    // Play using aplay in background (suppress output)
    std::string cmd = "aplay -q \"" + fullPath + "\" > /dev/null 2>&1 &";
    system(cmd.c_str());
}

void AudioEngine::SetMasterVolume(float volume) {
    // Clamp to 0-1
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    
    // Set system volume using amixer
    int percent = (int)(volume * 100);
    std::string cmd = "amixer -q set Master " + std::to_string(percent) + "%";
    system(cmd.c_str());
}

} // namespace vge