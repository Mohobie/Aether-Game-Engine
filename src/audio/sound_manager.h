#pragma once
#include <string>
#include <unordered_map>

namespace VoxelEngine {
    class SoundManager {
    public:
        void LoadSound(const std::string& name, const std::string& path);
        void PlaySound(const std::string& name);
        void StopSound(const std::string& name);
        void SetMasterVolume(float volume);
    private:
        std::unordered_map<std::string, int> soundBuffers;
    };
}
