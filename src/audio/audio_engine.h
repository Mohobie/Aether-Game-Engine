#pragma once
#include <string>

namespace vge {
class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();
    bool Initialize();
    void Shutdown();
    void PlaySound(const std::string& file);
    void SetMasterVolume(float volume);
    
    bool IsInitialized() const { return initialized; }
    void SetVolume(float volume) { SetMasterVolume(volume); }
    
private:
    bool initialized = false;
};
}