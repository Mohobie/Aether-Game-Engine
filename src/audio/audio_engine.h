#pragma once

#include <string>

namespace VoxelEngine {
    class AudioEngine {
    public:
        void Initialize();
        void PlaySound(const std::string& path);
        void PlayMusic(const std::string& path);
        void Shutdown();
    };
}
