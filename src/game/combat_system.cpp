#include "game/combat_system.h"
#include <cmath>
#include <iostream>
#include <algorithm>

namespace vge {

// ============================================
// CombatSystem Implementation
// ============================================

CombatSystem::CombatSystem()
    : enemySpawner(nullptr)
    , playerInventory(nullptr)
    , respawnTimer(0.0f)
    , respawnDelay(3.0f)
    , waitingForRespawn(false)
{
    // Default drop tables
    zombieDrops.emplace_back("rotten_flesh", 1, 3, 0.8f);
    zombieDrops.emplace_back("bone", 0, 2, 0.3f);
    
    skeletonDrops.emplace_back("bone", 1, 3, 0.8f);
    skeletonDrops.emplace_back("arrow", 0, 2, 0.5f);
}

void CombatSystem::Update(float deltaTime, const Vec3& playerPos) {
    // Update player attack cooldown
    if (playerStats.attackCooldownTimer > 0.0f) {
        playerStats.attackCooldownTimer -= deltaTime;
    }
    
    // Process enemy attacks on player
    if (playerStats.isAlive) {
        ProcessEnemyAttacks(playerPos);
    }
    
    // Handle respawn timer
    if (waitingForRespawn) {
        respawnTimer -= deltaTime;
        if (respawnTimer <= 0.0f) {
            RespawnPlayer();
        }
    }
}

bool CombatSystem::PlayerAttack(const Vec3& playerPos, const Vec3& playerForward) {
    if (!playerStats.isAlive || playerStats.attackCooldownTimer > 0.0f) {
        return false;
    }
    
    if (!enemySpawner) {
        return false;
    }
    
    // Find enemies in attack range
    std::vector<EnemyAI*> enemies = enemySpawner->GetEnemiesInRadius(playerPos, playerStats.attackRange);
    
    // Find the enemy closest to the forward direction
    EnemyAI* target = nullptr;
    float bestDot = -1.0f;
    
    for (EnemyAI* enemy : enemies) {
        if (!enemy || enemy->IsDead()) continue;
        
        Vec3 toEnemy = enemy->GetPosition() - playerPos;
        float dist = toEnemy.length();
        if (dist > playerStats.attackRange || dist < 0.1f) continue;
        
        toEnemy = toEnemy / dist; // Normalize
        float dot = playerForward.dot(toEnemy);
        
        // Must be within 90 degrees of forward direction (dot > 0)
        if (dot > 0.0f && dot > bestDot) {
            bestDot = dot;
            target = enemy;
        }
    }
    
    if (target) {
        return PlayerAttackEnemy(target, playerPos);
    }
    
    return false;
}

bool CombatSystem::PlayerAttackEnemy(EnemyAI* enemy, const Vec3& playerPos) {
    if (!enemy || enemy->IsDead() || !playerStats.isAlive) {
        return false;
    }
    
    // Check range
    float dist = (enemy->GetPosition() - playerPos).length();
    if (dist > playerStats.attackRange) {
        return false;
    }
    
    // Start cooldown
    playerStats.attackCooldownTimer = playerStats.attackCooldown;
    
    // Calculate damage
    DamageInfo damage = CalculateDamage(playerStats.attackDamage, 0.0f, false);
    
    // Apply damage to enemy
    enemy->TakeDamage(damage.finalDamage);
    
    if (onEnemyHit) {
        onEnemyHit(enemy);
    }
    
    std::cout << "[Combat] Player hits " 
              << (enemy->GetType() == EnemyType::ZOMBIE ? "Zombie" : "Skeleton")
              << " for " << damage.finalDamage << " damage!" << std::endl;
    
    // Check if enemy died
    if (enemy->IsDead()) {
        HandleEnemyDeath(enemy);
    }
    
    return true;
}

void CombatSystem::EnemyAttackPlayer(EnemyAI* enemy) {
    if (!enemy || enemy->IsDead() || !playerStats.isAlive) {
        return;
    }
    
    // Calculate damage with armor reduction
    DamageInfo damage = CalculateDamage(enemy->GetDamage(), playerStats.armor, 
                                         enemy->GetType() == EnemyType::SKELETON);
    
    // Apply damage to player
    DamagePlayer(damage.finalDamage);
    
    std::cout << "[Combat] " 
              << (enemy->GetType() == EnemyType::ZOMBIE ? "Zombie" : "Skeleton")
              << " hits player for " << damage.finalDamage << " damage!"
              << " (blocked " << damage.armorBlocked << " with armor)" << std::endl;
}

void CombatSystem::ProcessEnemyAttacks(const Vec3& playerPos) {
    if (!enemySpawner || !playerStats.isAlive) {
        return;
    }
    
    std::vector<EnemyAI*> enemies = enemySpawner->GetAllEnemies();
    
    for (EnemyAI* enemy : enemies) {
        if (!enemy || enemy->IsDead()) continue;
        
        float dist = (enemy->GetPosition() - playerPos).length();
        
        // Check if enemy can attack
        if (dist <= enemy->GetAttackRange() && enemy->CanAttack()) {
            EnemyAttackPlayer(enemy);
            enemy->ResetAttackCooldown();
        }
    }
}

DamageInfo CombatSystem::CalculateDamage(float baseDamage, float armor, bool isRanged) {
    DamageInfo info;
    info.baseDamage = baseDamage;
    info.isRanged = isRanged;
    info.damageType = isRanged ? "ranged" : "melee";
    
    // Critical hit chance (10%)
    info.isCritical = (rand() % 100) < 10;
    if (info.isCritical) {
        baseDamage *= 1.5f;
    }
    
    // Apply armor reduction
    info.armorBlocked = ApplyArmorReduction(baseDamage, armor);
    info.finalDamage = std::max(1.0f, baseDamage - info.armorBlocked);
    
    return info;
}

float CombatSystem::ApplyArmorReduction(float damage, float armor) {
    // Armor reduces damage by a percentage based on armor value
    // Each point of armor blocks 4% of damage, capped at 80%
    float reductionPercent = std::min(0.8f, armor * 0.04f);
    return damage * reductionPercent;
}

void CombatSystem::DamagePlayer(float amount) {
    if (!playerStats.isAlive) return;
    
    playerStats.health -= amount;
    
    if (onPlayerDamaged) {
        onPlayerDamaged(amount);
    }
    
    if (playerStats.health <= 0.0f) {
        playerStats.health = 0.0f;
        KillPlayer();
    }
}

void CombatSystem::HealPlayer(float amount) {
    if (!playerStats.isAlive) return;
    
    playerStats.health = std::min(playerStats.maxHealth, playerStats.health + amount);
}

void CombatSystem::KillPlayer() {
    if (!playerStats.isAlive) return;
    
    playerStats.isAlive = false;
    waitingForRespawn = true;
    respawnTimer = respawnDelay;
    
    if (onPlayerDeath) {
        onPlayerDeath();
    }
    
    std::cout << "[Combat] Player died! Respawning in " << respawnDelay << " seconds..." << std::endl;
}

void CombatSystem::RespawnPlayer() {
    playerStats.health = playerStats.maxHealth;
    playerStats.isAlive = true;
    waitingForRespawn = false;
    respawnTimer = 0.0f;
    
    if (onPlayerRespawn) {
        onPlayerRespawn();
    }
    
    std::cout << "[Combat] Player respawned at (" 
              << playerStats.respawnPosition.x << ", "
              << playerStats.respawnPosition.y << ", "
              << playerStats.respawnPosition.z << ")" << std::endl;
}

void CombatSystem::HandleEnemyDeath(EnemyAI* enemy) {
    if (!enemy) return;
    
    // Roll drops
    auto drops = RollDrops(enemy->GetType());
    
    // Spawn drops
    if (!drops.empty()) {
        SpawnDrops(enemy->GetPosition(), drops);
    }
    
    if (onEnemyDied) {
        onEnemyDied(enemy, drops);
    }
    
    std::cout << "[Combat] " 
              << (enemy->GetType() == EnemyType::ZOMBIE ? "Zombie" : "Skeleton")
              << " died!" << std::endl;
}

std::vector<std::pair<std::string, int>> CombatSystem::RollDrops(EnemyType type) {
    std::vector<std::pair<std::string, int>> result;
    const std::vector<DropEntry>* dropTable = nullptr;
    
    switch (type) {
        case EnemyType::ZOMBIE:
            dropTable = &zombieDrops;
            break;
        case EnemyType::SKELETON:
            dropTable = &skeletonDrops;
            break;
    }
    
    if (!dropTable) return result;
    
    for (const auto& entry : *dropTable) {
        float roll = static_cast<float>(rand() % 100) / 100.0f;
        if (roll <= entry.dropChance) {
            int count = entry.minCount + (rand() % (entry.maxCount - entry.minCount + 1));
            if (count > 0) {
                result.emplace_back(entry.itemID, count);
            }
        }
    }
    
    return result;
}

void CombatSystem::SpawnDrops(const Vec3& position, const std::vector<std::pair<std::string, int>>& drops) {
    for (const auto& drop : drops) {
        std::cout << "[Combat] Dropped " << drop.second << "x " << drop.first 
                  << " at (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
        
        // In a full implementation, this would create physical drop entities
        // For now, we just log and optionally add to player inventory if close
        if (playerInventory) {
            // Could add to inventory if player is close
            // playerInventory->AddItem(drop.first, drop.second);
        }
    }
}

void CombatSystem::AddZombieDrop(const std::string& itemID, int minCount, int maxCount, float chance) {
    zombieDrops.emplace_back(itemID, minCount, maxCount, chance);
}

void CombatSystem::AddSkeletonDrop(const std::string& itemID, int minCount, int maxCount, float chance) {
    skeletonDrops.emplace_back(itemID, minCount, maxCount, chance);
}

void CombatSystem::ClearDropTables() {
    zombieDrops.clear();
    skeletonDrops.clear();
}

} // namespace vge
