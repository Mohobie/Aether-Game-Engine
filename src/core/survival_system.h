#pragma once
#include "math/vec3.h"
#include <functional>

namespace vge {

// ============================================
// Survival System - Health, Hunger, Damage
// ============================================
class SurvivalSystem {
public:
    SurvivalSystem();
    ~SurvivalSystem();

    // Update (call every frame)
    void Update(float deltaTime);

    // Health
    float GetHealth() const { return health; }
    float GetMaxHealth() const { return maxHealth; }
    void SetHealth(float value);
    void TakeDamage(float amount);
    void Heal(float amount);
    bool IsDead() const { return health <= 0.0f; }
    void Respawn(const Vec3& spawnPos);
    void Die();

    // Hunger
    float GetHunger() const { return hunger; }
    float GetMaxHunger() const { return maxHunger; }
    void SetHunger(float value);
    void Eat(float amount);
    void Starve(float amount);
    bool IsStarving() const { return hunger <= 0.0f; }

    // Breathing (for water)
    float GetBreath() const { return breath; }
    float GetMaxBreath() const { return maxBreath; }
    void SetBreath(float value);
    bool IsDrowning() const { return breath <= 0.0f; }
    void SetUnderwater(bool underwater);

    // Experience/Levels
    int GetLevel() const { return level; }
    float GetExperience() const { return experience; }
    void AddExperience(float amount);

    // Death/Respawn
    Vec3 GetSpawnPoint() const { return spawnPoint; }
    void SetSpawnPoint(const Vec3& pos) { spawnPoint = pos; }
    int GetDeathCount() const { return deathCount; }

    // Callbacks
    std::function<void()> onDeath;
    std::function<void()> onRespawn;
    std::function<void(float)> onHealthChanged;
    std::function<void(float)> onHungerChanged;
    std::function<void(int)> onLevelUp;

private:
    // Health
    float health;
    float maxHealth;
    float healthRegenRate;
    float healthRegenDelay;
    float healthRegenTimer;

    // Hunger
    float hunger;
    float maxHunger;
    float hungerDecayRate;
    float hungerDamageRate;
    float hungerDamageTimer;

    // Breath (for drowning)
    float breath;
    float maxBreath;
    float breathDecayRate;
    float breathRegenRate;
    bool isUnderwater;

    // Experience
    float experience;
    int level;
    float expToNextLevel;

    // Spawn/Death
    Vec3 spawnPoint;
    int deathCount;
    float respawnInvulnerabilityTime;
    float invulnerabilityTimer;

    void UpdateHealthRegen(float deltaTime);
    void UpdateHunger(float deltaTime);
    void UpdateBreath(float deltaTime);
    void CheckLevelUp();
};

} // namespace vge
