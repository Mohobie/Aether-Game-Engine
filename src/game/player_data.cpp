#include "player_data.h"
#include <fstream>
#include <iostream>
#include <cstring>

namespace vge {

PlayerData::PlayerData()
    : position(Vec3(0, 100, 0))
    , rotation(Vec3(0, 0, 0))
    , velocity(Vec3(0, 0, 0))
    , health(100.0f)
    , hunger(100.0f)
    , stamina(100.0f)
    , maxHealth(100.0f)
    , maxHunger(100.0f)
    , maxStamina(100.0f)
    , selectedSlot(0)
    , inventorySize(DEFAULT_INVENTORY_SIZE)
    , spawnPoint(Vec3(0, 100, 0))
    , playTime(0.0f)
    , blocksMined(0)
    , blocksPlaced(0)
    , enemiesKilled(0)
    , deaths(0)
{
    inventorySlots.resize(inventorySize);
}

void PlayerData::FromPlayer(const Vec3& pos, const Vec3& rot, const Vec3& vel,
                            const PlayerStats& stats, const Inventory& inventory) {
    position = pos;
    rotation = rot;
    velocity = vel;

    health = stats.GetHealth();
    hunger = stats.GetHunger();
    stamina = stats.GetStamina();
    maxHealth = stats.GetMaxHealth();
    maxHunger = stats.GetMaxHunger();
    maxStamina = stats.GetMaxStamina();

    spawnPoint = stats.GetSpawnPoint();

    selectedSlot = inventory.GetSelectedSlot();
    inventorySize = inventory.GetSize();
    inventorySlots.resize(inventorySize);
    for (int i = 0; i < inventorySize; ++i) {
        inventorySlots[i] = inventory.GetSlot(i);
    }
}

void PlayerData::ToPlayer(Vec3& outPos, Vec3& outRot, Vec3& outVel,
                          PlayerStats& outStats, Inventory& outInventory) const {
    outPos = position;
    outRot = rotation;
    outVel = velocity;

    outStats.SetHealth(health);
    outStats.SetHunger(hunger);
    outStats.SetStamina(stamina);
    outStats.SetSpawnPoint(spawnPoint);

    outInventory = Inventory(inventorySize);
    for (int i = 0; i < inventorySize && i < (int)inventorySlots.size(); ++i) {
        if (inventorySlots[i].type != BLOCK_AIR && inventorySlots[i].count > 0) {
            outInventory.AddItem(inventorySlots[i].type, inventorySlots[i].count);
        }
    }
    outInventory.SelectSlot(selectedSlot);
}

std::vector<uint8_t> PlayerData::Serialize() const {
    std::vector<uint8_t> data;

    // Helper lambda to append raw bytes
    auto append = [&data](const void* ptr, size_t size) {
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(ptr);
        data.insert(data.end(), bytes, bytes + size);
    };

    // Version
    uint32_t version = 1;
    append(&version, sizeof(version));

    // Position
    append(&position.x, sizeof(float));
    append(&position.y, sizeof(float));
    append(&position.z, sizeof(float));

    // Rotation
    append(&rotation.x, sizeof(float));
    append(&rotation.y, sizeof(float));
    append(&rotation.z, sizeof(float));

    // Velocity
    append(&velocity.x, sizeof(float));
    append(&velocity.y, sizeof(float));
    append(&velocity.z, sizeof(float));

    // Stats
    append(&health, sizeof(float));
    append(&hunger, sizeof(float));
    append(&stamina, sizeof(float));
    append(&maxHealth, sizeof(float));
    append(&maxHunger, sizeof(float));
    append(&maxStamina, sizeof(float));

    // Spawn point
    append(&spawnPoint.x, sizeof(float));
    append(&spawnPoint.y, sizeof(float));
    append(&spawnPoint.z, sizeof(float));

    // Inventory
    append(&inventorySize, sizeof(int));
    append(&selectedSlot, sizeof(int));

    uint32_t slotCount = static_cast<uint32_t>(inventorySlots.size());
    append(&slotCount, sizeof(uint32_t));

    for (const auto& slot : inventorySlots) {
        append(&slot.type, sizeof(BlockTypeID));
        append(&slot.count, sizeof(int));
    }

    // Unlocked recipes
    uint32_t recipeCount = static_cast<uint32_t>(unlockedRecipes.size());
    append(&recipeCount, sizeof(uint32_t));
    for (const auto& recipe : unlockedRecipes) {
        uint32_t len = static_cast<uint32_t>(recipe.length());
        append(&len, sizeof(uint32_t));
        data.insert(data.end(), recipe.begin(), recipe.end());
    }

    // Stats
    append(&playTime, sizeof(float));
    append(&blocksMined, sizeof(int));
    append(&blocksPlaced, sizeof(int));
    append(&enemiesKilled, sizeof(int));
    append(&deaths, sizeof(int));

    return data;
}

bool PlayerData::Deserialize(const std::vector<uint8_t>& data) {
    if (data.size() < sizeof(uint32_t)) return false;

    size_t offset = 0;

    auto read = [&data, &offset](void* ptr, size_t size) -> bool {
        if (offset + size > data.size()) return false;
        std::memcpy(ptr, data.data() + offset, size);
        offset += size;
        return true;
    };

    // Version
    uint32_t version;
    if (!read(&version, sizeof(version))) return false;
    if (version != 1) {
        std::cerr << "[PlayerData] Unknown version: " << version << std::endl;
        return false;
    }

    // Position
    if (!read(&position.x, sizeof(float))) return false;
    if (!read(&position.y, sizeof(float))) return false;
    if (!read(&position.z, sizeof(float))) return false;

    // Rotation
    if (!read(&rotation.x, sizeof(float))) return false;
    if (!read(&rotation.y, sizeof(float))) return false;
    if (!read(&rotation.z, sizeof(float))) return false;

    // Velocity
    if (!read(&velocity.x, sizeof(float))) return false;
    if (!read(&velocity.y, sizeof(float))) return false;
    if (!read(&velocity.z, sizeof(float))) return false;

    // Stats
    if (!read(&health, sizeof(float))) return false;
    if (!read(&hunger, sizeof(float))) return false;
    if (!read(&stamina, sizeof(float))) return false;
    if (!read(&maxHealth, sizeof(float))) return false;
    if (!read(&maxHunger, sizeof(float))) return false;
    if (!read(&maxStamina, sizeof(float))) return false;

    // Spawn point
    if (!read(&spawnPoint.x, sizeof(float))) return false;
    if (!read(&spawnPoint.y, sizeof(float))) return false;
    if (!read(&spawnPoint.z, sizeof(float))) return false;

    // Inventory
    if (!read(&inventorySize, sizeof(int))) return false;
    if (!read(&selectedSlot, sizeof(int))) return false;

    uint32_t slotCount;
    if (!read(&slotCount, sizeof(uint32_t))) return false;

    inventorySlots.resize(slotCount);
    for (uint32_t i = 0; i < slotCount; ++i) {
        if (!read(&inventorySlots[i].type, sizeof(BlockTypeID))) return false;
        if (!read(&inventorySlots[i].count, sizeof(int))) return false;
    }

    // Unlocked recipes
    uint32_t recipeCount;
    if (!read(&recipeCount, sizeof(uint32_t))) return false;
    unlockedRecipes.clear();
    for (uint32_t i = 0; i < recipeCount; ++i) {
        uint32_t len;
        if (!read(&len, sizeof(uint32_t))) return false;
        if (offset + len > data.size()) return false;
        std::string recipe(data.begin() + offset, data.begin() + offset + len);
        offset += len;
        unlockedRecipes.insert(recipe);
    }

    // Stats
    if (!read(&playTime, sizeof(float))) return false;
    if (!read(&blocksMined, sizeof(int))) return false;
    if (!read(&blocksPlaced, sizeof(int))) return false;
    if (!read(&enemiesKilled, sizeof(int))) return false;
    if (!read(&deaths, sizeof(int))) return false;

    return true;
}

bool PlayerData::SaveToFile(const std::string& path) const {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[PlayerData] Failed to open: " << path << std::endl;
        return false;
    }

    auto data = Serialize();
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    file.close();

    return true;
}

bool PlayerData::LoadFromFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[PlayerData] Failed to open: " << path << std::endl;
        return false;
    }

    std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
    file.close();

    return Deserialize(data);
}

bool PlayerData::IsValid() const {
    return health >= 0.0f && health <= maxHealth &&
           hunger >= 0.0f && hunger <= maxHunger &&
           stamina >= 0.0f && stamina <= maxStamina &&
           inventorySize > 0 && inventorySize <= 100;
}

void PlayerData::Reset() {
    position = Vec3(0, 100, 0);
    rotation = Vec3(0, 0, 0);
    velocity = Vec3(0, 0, 0);
    health = 100.0f;
    hunger = 100.0f;
    stamina = 100.0f;
    maxHealth = 100.0f;
    maxHunger = 100.0f;
    maxStamina = 100.0f;
    selectedSlot = 0;
    inventorySize = DEFAULT_INVENTORY_SIZE;
    inventorySlots.clear();
    inventorySlots.resize(inventorySize);
    unlockedRecipes.clear();
    spawnPoint = Vec3(0, 100, 0);
    playTime = 0.0f;
    blocksMined = 0;
    blocksPlaced = 0;
    enemiesKilled = 0;
    deaths = 0;
}

} // namespace vge
