#include "key_bindings.h"
#include "core/logger.h"
#include "platform/input.h"
#include <fstream>
#include <sstream>

namespace vge {

KeyBindings::KeyBindings() {
    SetDefaults();
}

KeyBindings::~KeyBindings() {
}

void KeyBindings::SetDefaults() {
    // Movement
    bindings["move_forward"] = GLFW_KEY_W;
    bindings["move_backward"] = GLFW_KEY_S;
    bindings["move_left"] = GLFW_KEY_A;
    bindings["move_right"] = GLFW_KEY_D;
    bindings["jump"] = GLFW_KEY_SPACE;
    bindings["sneak"] = GLFW_KEY_LEFT_SHIFT;
    bindings["sprint"] = GLFW_KEY_LEFT_CONTROL;
    
    // Actions
    bindings["attack"] = GLFW_KEY_Q;
    bindings["use"] = GLFW_KEY_E;
    bindings["drop"] = GLFW_KEY_G;
    bindings["inventory"] = GLFW_KEY_I;
    bindings["crafting"] = GLFW_KEY_C;
    
    // Hotbar
    bindings["hotbar_1"] = GLFW_KEY_1;
    bindings["hotbar_2"] = GLFW_KEY_2;
    bindings["hotbar_3"] = GLFW_KEY_3;
    bindings["hotbar_4"] = GLFW_KEY_4;
    bindings["hotbar_5"] = GLFW_KEY_5;
    bindings["hotbar_6"] = GLFW_KEY_6;
    bindings["hotbar_7"] = GLFW_KEY_7;
    bindings["hotbar_8"] = GLFW_KEY_8;
    bindings["hotbar_9"] = GLFW_KEY_9;
    
    // UI
    bindings["pause"] = GLFW_KEY_ESCAPE;
    bindings["debug"] = GLFW_KEY_F3;
    bindings["fullscreen"] = GLFW_KEY_F11;
    bindings["screenshot"] = GLFW_KEY_F2;
}

bool KeyBindings::LoadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        Logger::Info("[KeyBindings] No key bindings file found, using defaults");
        return false;
    }
    
    bindings.clear();
    std::string line;
    while (std::getline(file, line)) {
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string action = line.substr(0, pos);
            int keyCode = std::stoi(line.substr(pos + 1));
            bindings[action] = keyCode;
        }
    }
    
    file.close();
    Logger::Info("[KeyBindings] Loaded key bindings from " + filepath);
    return true;
}

bool KeyBindings::SaveToFile(const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        Logger::Error("[KeyBindings] Failed to save key bindings to " + filepath);
        return false;
    }
    
    for (const auto& [action, keyCode] : bindings) {
        file << action << "=" << keyCode << "\n";
    }
    
    file.close();
    Logger::Info("[KeyBindings] Saved key bindings to " + filepath);
    return true;
}

int KeyBindings::GetKey(const std::string& action) const {
    auto it = bindings.find(action);
    if (it != bindings.end()) {
        return it->second;
    }
    return -1;
}

void KeyBindings::SetKey(const std::string& action, int keyCode) {
    bindings[action] = keyCode;
    Logger::Info("[KeyBindings] Bound " + action + " to key " + std::to_string(keyCode));
}

void KeyBindings::ResetToDefaults() {
    bindings.clear();
    SetDefaults();
    Logger::Info("[KeyBindings] Reset to default bindings");
}

bool KeyBindings::IsBound(const std::string& action) const {
    return bindings.find(action) != bindings.end();
}

std::unordered_map<std::string, int> KeyBindings::GetAllBindings() const {
    return bindings;
}

} // namespace vge
