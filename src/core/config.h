#pragma once
#include <string>
#include <unordered_map>

namespace vge {

class Config {
public:
    // Graphics
    int renderDistance;
    bool vsync;
    bool fullscreen;
    float fov;
    bool showFPS;
    
    // Audio
    float masterVolume;
    float musicVolume;
    float sfxVolume;
    
    // Controls
    float mouseSensitivity;
    std::unordered_map<std::string, int> keybinds;
    
    Config();
    
    bool Load(const std::string& path);
    bool Save(const std::string& path) const;
    void SetDefaultKeybinds();
};

} // namespace vge
