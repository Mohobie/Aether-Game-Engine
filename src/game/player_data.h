#pragma once

#include "math/vec3.h"
#include "core/inventory.h"
#include "game/player_stats.h"
#include <cstdint>
#include <string>
#include <vector>
#include <set>

namespace vge {

// ============================================
// PlayerData - Serializable player state
// ============================================
struct PlayerData {
    // Position & rotation
    Vec3 position;
    Vec3 rotation;    // Euler angles (pitch, yaw, roll)
    Vec3 velocity;

    // Stats
    float health;
    float hunger;
    float stamina;
    float maxHealth;
    float maxHunger;
    float maxStamina;

    // Inventory
    std::vector<InventorySlot> inventorySlots;
    int selectedSlot;
    int inventorySize;

    // Crafting
    std::set<std::string> unlockedRecipes;

    // Spawn point
    Vec3 spawnPoint;

    // Metadata
    float playTime;       // Total play time in seconds
    int blocksMined;
    int blocksPlaced;
    int enemiesKilled;
    int deaths;

    PlayerData();

    // Initialize from current game state
    void FromPlayer(const Vec3& pos, const Vec3& rot, const Vec3& vel,
                    const PlayerStats& stats, const Inventory& inventory);

    // Apply to game state
    void ToPlayer(Vec3& outPos, Vec3& outRot, Vec3& outVel,
                  PlayerStats& outStats, Inventory& outInventory) const;

    // Serialization
    std::vector<uint8_t> Serialize() const;
    bool Deserialize(const std::vector<uint8_t>& data);

    // File I/O
    bool SaveToFile(const std::string& path) const;
    bool LoadFromFile(const std::string& path);

    // Validation
    bool IsValid() const;
    void Reset();
};

} // namespace vge
