#pragma once
#include <string>
#include <unordered_map>
#include "voxel/block.h"

namespace vge {

class AudioEngine;

class SoundManager {
private:
    std::unordered_map<std::string, std::string> sounds;
    AudioEngine* audioEngine;
    
public:
    SoundManager();
    ~SoundManager();
    
    bool Initialize(AudioEngine* engine);
    void Shutdown();
    
    bool LoadSound(const std::string& name, const std::string& path);
    void PlaySound(const std::string& name, float volume = 1.0f, bool loop = false);
    void StopSound(const std::string& name);
    void StopAllSounds();
    
    void SetMasterVolume(float volume);
    
    // Convenience methods for game events
    void PlayBlockPlace(BlockType type);
    void PlayBlockBreak(BlockType type);
    void PlayFootstep(BlockType groundType);
    void PlayJump();
    void PlayLand();
};

} // namespace vge
