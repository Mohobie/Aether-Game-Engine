#pragma once
#include <string>

namespace vge {
class AudioEngine {
public:
    bool Initialize();
    void Shutdown();
    void PlaySound(const std::string& file);
    void SetMasterVolume(float volume);
};
}