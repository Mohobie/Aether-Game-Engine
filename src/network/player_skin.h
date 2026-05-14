#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace vge {

// ============================================
// Player Skin
// ============================================
struct PlayerSkin {
    std::string name;
    std::vector<uint8_t> textureData; // RGBA pixel data
    int width;
    int height;
    
    PlayerSkin() : width(64), height(64) {}
};

// ============================================
// Skin Manager
// ============================================
class SkinManager {
public:
    SkinManager();
    ~SkinManager();

    // Load skin from file
    bool LoadSkin(const std::string& name, const std::string& filepath);
    
    // Create default skin
    void CreateDefaultSkin(const std::string& name);
    
    // Get skin
    PlayerSkin* GetSkin(const std::string& name);
    
    // Set player skin
    void SetPlayerSkin(uint32_t playerId, const std::string& skinName);
    std::string GetPlayerSkin(uint32_t playerId) const;
    
    // Get all skin names
    std::vector<std::string> GetSkinNames() const;

private:
    std::unordered_map<std::string, PlayerSkin> skins;
    std::unordered_map<uint32_t, std::string> playerSkins;
};

} // namespace vge
