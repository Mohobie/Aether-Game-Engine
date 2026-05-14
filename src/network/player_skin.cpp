#include "player_skin.h"
#include "core/logger.h"

namespace vge {

SkinManager::SkinManager() {
    // Create default skins
    CreateDefaultSkin("default");
    CreateDefaultSkin("steve");
    CreateDefaultSkin("alex");
}

SkinManager::~SkinManager() {
}

bool SkinManager::LoadSkin(const std::string& name, const std::string& filepath) {
    // In a real implementation, this would load from PNG file
    Logger::Info("[SkinManager] Loading skin: " + name + " from " + filepath);
    
    PlayerSkin skin;
    skin.name = name;
    skin.width = 64;
    skin.height = 64;
    skin.textureData.resize(64 * 64 * 4, 255); // White placeholder
    
    skins[name] = skin;
    return true;
}

void SkinManager::CreateDefaultSkin(const std::string& name) {
    PlayerSkin skin;
    skin.name = name;
    skin.width = 64;
    skin.height = 64;
    skin.textureData.resize(64 * 64 * 4, 255); // White placeholder
    
    // Add some color variation based on name
    if (name == "steve") {
        // Brown hair, blue shirt, blue pants
        for (int i = 0; i < 64 * 64 * 4; i += 4) {
            skin.textureData[i] = 200;   // R
            skin.textureData[i+1] = 150; // G
            skin.textureData[i+2] = 100; // B
        }
    } else if (name == "alex") {
        // Orange hair, green shirt, dark pants
        for (int i = 0; i < 64 * 64 * 4; i += 4) {
            skin.textureData[i] = 255;   // R
            skin.textureData[i+1] = 200; // G
            skin.textureData[i+2] = 150; // B
        }
    }
    
    skins[name] = skin;
    Logger::Info("[SkinManager] Created default skin: " + name);
}

PlayerSkin* SkinManager::GetSkin(const std::string& name) {
    auto it = skins.find(name);
    if (it != skins.end()) {
        return &(it->second);
    }
    return nullptr;
}

void SkinManager::SetPlayerSkin(uint32_t playerId, const std::string& skinName) {
    playerSkins[playerId] = skinName;
    Logger::Info("[SkinManager] Set player " + std::to_string(playerId) + " skin to " + skinName);
}

std::string SkinManager::GetPlayerSkin(uint32_t playerId) const {
    auto it = playerSkins.find(playerId);
    if (it != playerSkins.end()) {
        return it->second;
    }
    return "default";
}

std::vector<std::string> SkinManager::GetSkinNames() const {
    std::vector<std::string> names;
    for (const auto& [name, skin] : skins) {
        names.push_back(name);
    }
    return names;
}

} // namespace vge
