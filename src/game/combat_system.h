#pragma once
#include "math/vec3.h"
#include "ai/enemy_ai.h"
#include "ai/enemy_spawner.h"
#include "core/item_system.h"
#include <vector>
#include <string>
#include <functional>

namespace vge {

// ============================================
// Player Combat Stats
// ============================================
struct PlayerCombatStats {
    float health;
    float maxHealth;
    float armor;
    float armorReduction; // 0-1, percentage
    float attackDamage;
    float attackRange;
    float attackCooldown;
    float attackCooldownTimer;
    bool isAlive;
    Vec3 respawnPosition;
    
    PlayerCombatStats()
        : health(100.0f)
        , maxHealth(100.0f)
        , armor(0.0f)
        , armorReduction(0.0f)
        , attackDamage(20.0f)
        , attackRange(2.0f)
        , attackCooldown(0.5f)
        , attackCooldownTimer(0.0f)
        , isAlive(true)
        , respawnPosition(0, 20, 0)
    {}
};

// ============================================
// Damage Info
// ============================================
struct DamageInfo {
    float baseDamage;
    float finalDamage;
    float armorBlocked;
    bool isCritical;
    bool isRanged;
    Vec3 sourcePosition;
    Vec3 targetPosition;
    std::string damageType; // "melee", "ranged", "magic"
    
    DamageInfo()
        : baseDamage(0.0f)
        , finalDamage(0.0f)
        , armorBlocked(0.0f)
        , isCritical(false)
        , isRanged(false)
        , damageType("melee")
    {}
};

// ============================================
// Drop Table Entry
// ============================================
struct DropEntry {
    std::string itemID;
    int minCount;
    int maxCount;
    float dropChance; // 0-1 probability
    
    DropEntry(const std::string& id, int minC, int maxC, float chance)
        : itemID(id), minCount(minC), maxCount(maxC), dropChance(chance) {}
};

// ============================================
// Combat System
// ============================================
class CombatSystem {
private:
    PlayerCombatStats playerStats;
    EnemySpawner* enemySpawner;
    Inventory* playerInventory;
    
    // Drop tables per enemy type
    std::vector<DropEntry> zombieDrops;
    std::vector<DropEntry> skeletonDrops;
    
    // Death handling
    float respawnTimer;
    float respawnDelay;
    bool waitingForRespawn;
    
public:
    CombatSystem();
    
    // Update
    void Update(float deltaTime, const Vec3& playerPos);
    
    // Player attacks
    bool PlayerAttack(const Vec3& playerPos, const Vec3& playerForward);
    bool PlayerAttackEnemy(EnemyAI* enemy, const Vec3& playerPos);
    
    // Enemy attacks player
    void EnemyAttackPlayer(EnemyAI* enemy);
    void ProcessEnemyAttacks(const Vec3& playerPos);
    
    // Damage calculation
    DamageInfo CalculateDamage(float baseDamage, float armor, bool isRanged = false);
    float ApplyArmorReduction(float damage, float armor);
    
    // Player damage
    void DamagePlayer(float amount);
    void HealPlayer(float amount);
    void KillPlayer();
    void RespawnPlayer();
    
    // Death and drops
    void HandleEnemyDeath(EnemyAI* enemy);
    std::vector<std::pair<std::string, int>> RollDrops(EnemyType type);
    void SpawnDrops(const Vec3& position, const std::vector<std::pair<std::string, int>>& drops);
    
    // Getters
    PlayerCombatStats& GetPlayerStats() { return playerStats; }
    const PlayerCombatStats& GetPlayerStats() const { return playerStats; }
    float GetPlayerHealth() const { return playerStats.health; }
    float GetPlayerHealthPercent() const { return playerStats.health / playerStats.maxHealth; }
    bool IsPlayerAlive() const { return playerStats.isAlive; }
    bool IsWaitingForRespawn() const { return waitingForRespawn; }
    
    // Setters
    void SetEnemySpawner(EnemySpawner* spawner) { enemySpawner = spawner; }
    void SetPlayerInventory(Inventory* inv) { playerInventory = inv; }
    void SetPlayerPosition(Vec3& pos) { playerStats.respawnPosition = pos; }
    
    // Configuration
    void SetPlayerMaxHealth(float health) { playerStats.maxHealth = health; playerStats.health = health; }
    void SetPlayerArmor(float armor) { playerStats.armor = armor; }
    void SetPlayerDamage(float damage) { playerStats.attackDamage = damage; }
    void SetRespawnDelay(float delay) { respawnDelay = delay; }
    
    // Drop table configuration
    void AddZombieDrop(const std::string& itemID, int minCount, int maxCount, float chance);
    void AddSkeletonDrop(const std::string& itemID, int minCount, int maxCount, float chance);
    void ClearDropTables();
    
    // Callbacks
    std::function<void(float)> onPlayerDamaged;
    std::function<void()> onPlayerDeath;
    std::function<void()> onPlayerRespawn;
    std::function<void(EnemyAI*, const std::vector<std::pair<std::string, int>>&)> onEnemyDied;
    std::function<void(EnemyAI*)> onEnemyHit;
};

} // namespace vge
