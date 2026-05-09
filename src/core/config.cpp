#include "config.h"
#include "json.hpp"
#include <fstream>
#include <iostream>

namespace vge {

using json = nlohmann::json;

Config::Config() : renderDistance(16), vsync(true), fullscreen(false),
    masterVolume(1.0f), musicVolume(0.7f), sfxVolume(1.0f),
    mouseSensitivity(1.0f), fov(70.0f), showFPS(true) {}

bool Config::Load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "[Config] No existing config, using defaults" << std::endl;
        return false;
    }
    
    try {
        json j;
        file >> j;
        
        // Graphics
        if (j.contains("graphics")) {
            auto& gfx = j["graphics"];
            renderDistance = gfx.value("renderDistance", 16);
            vsync = gfx.value("vsync", true);
            fullscreen = gfx.value("fullscreen", false);
            fov = gfx.value("fov", 70.0f);
            showFPS = gfx.value("showFPS", true);
        }
        
        // Audio
        if (j.contains("audio")) {
            auto& audio = j["audio"];
            masterVolume = audio.value("masterVolume", 1.0f);
            musicVolume = audio.value("musicVolume", 0.7f);
            sfxVolume = audio.value("sfxVolume", 1.0f);
        }
        
        // Controls
        if (j.contains("controls")) {
            auto& ctrl = j["controls"];
            mouseSensitivity = ctrl.value("mouseSensitivity", 1.0f);
        }
        
        std::cout << "[Config] Loaded from: " << path << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[Config] Error loading: " << e.what() << std::endl;
        return false;
    }
}

bool Config::Save(const std::string& path) const {
    try {
        json j;
        
        // Graphics
        j["graphics"]["renderDistance"] = renderDistance;
        j["graphics"]["vsync"] = vsync;
        j["graphics"]["fullscreen"] = fullscreen;
        j["graphics"]["fov"] = fov;
        j["graphics"]["showFPS"] = showFPS;
        
        // Audio
        j["audio"]["masterVolume"] = masterVolume;
        j["audio"]["musicVolume"] = musicVolume;
        j["audio"]["sfxVolume"] = sfxVolume;
        
        // Controls
        j["controls"]["mouseSensitivity"] = mouseSensitivity;
        
        std::ofstream file(path);
        if (!file.is_open()) {
            std::cerr << "[Config] Failed to open: " << path << std::endl;
            return false;
        }
        
        file << j.dump(4);
        std::cout << "[Config] Saved to: " << path << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[Config] Error saving: " << e.what() << std::endl;
        return false;
    }
}

void Config::SetDefaultKeybinds() {
    keybinds["forward"] = 87;    // W
    keybinds["backward"] = 83;   // S
    keybinds["left"] = 65;       // A
    keybinds["right"] = 68;      // D
    keybinds["jump"] = 32;       // Space
    keybinds["sneak"] = 340;     // Left Shift
    keybinds["inventory"] = 69;  // E
    keybinds["drop"] = 81;       // Q
    keybinds["attack"] = 0;      // Mouse left
    keybinds["use"] = 1;         // Mouse right
    keybinds["pause"] = 256;     // Escape
}

} // namespace vge