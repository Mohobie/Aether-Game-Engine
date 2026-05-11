#pragma once
#include "math/vec3.h"

namespace vge {

// ============================================
// Player Stats Component - Survival Mode
// ============================================
// Tracks health, hunger, and stamina for survival gameplay.
// All stats clamped to 0-100 range.
//
class PlayerStats {
private:
    float health;       // 0-100, death at <= 0
    float hunger;       // 0-100, affects health regen
    float stamina;      // 0-100, consumed by sprinting/actions
    float maxHealth;
    float maxHunger;
    float maxStamina;

    // State
    bool isDead;
    bool isSprinting;
    float hungerDecayTimer;
    float healthRegenTimer;

    // Configuration
    float hungerDecayRate;      // Hunger lost per second
    float staminaRegenRate;     // Stamina regained per second when not sprinting
    float staminaSprintCost;    // Stamina lost per second while sprinting
    float healthRegenRate;      // Health regained per second when well-fed
    float starvationDamage;     // Damage taken when hunger is 0

    // Spawn point for respawn
    Vec3 spawnPoint;

    // Internal helpers
    void ClampStats();

public:
    PlayerStats();
    explicit PlayerStats(const Vec3& spawn);

    // Core stat methods
    void TakeDamage(float amount);
    void Heal(float amount);
    void Eat(float foodValue);
    bool UseStamina(float amount);
    void RegenerateStamina(float amount);

    // Update (call each frame with deltaTime)
    void Update(float deltaTime);

    // Death & respawn
    bool IsDead() const { return isDead; }
    void Die();
    void Respawn();
    void SetSpawnPoint(const Vec3& point) { spawnPoint = point; }
    Vec3 GetSpawnPoint() const { return spawnPoint; }

    // Sprinting state
    void SetSprinting(bool sprinting) { isSprinting = sprinting; }
    bool IsSprinting() const { return isSprinting; }

    // Getters
    float GetHealth() const { return health; }
    float GetHunger() const { return hunger; }
    float GetStamina() const { return stamina; }
    float GetMaxHealth() const { return maxHealth; }
    float GetMaxHunger() const { return maxHunger; }
    float GetMaxStamina() const { return maxStamina; }

    // Setters (for initialization / cheats)
    void SetHealth(float value);
    void SetHunger(float value);
    void SetStamina(float value);

    // Configuration
    void SetHungerDecayRate(float rate) { hungerDecayRate = rate; }
    void SetStaminaRegenRate(float rate) { staminaRegenRate = rate; }
    void SetStaminaSprintCost(float cost) { staminaSprintCost = cost; }
    void SetHealthRegenRate(float rate) { healthRegenRate = rate; }
    void SetStarvationDamage(float damage) { starvationDamage = damage; }

    // Queries
    bool CanSprint() const { return stamina > staminaSprintCost && !isDead; }
    bool IsWellFed() const { return hunger > maxHunger * 0.8f; }
    bool IsStarving() const { return hunger <= 0.0f; }
    float GetHealthPercent() const { return health / maxHealth; }
    float GetHungerPercent() const { return hunger / maxHunger; }
    float GetStaminaPercent() const { return stamina / maxStamina; }
};

} // namespace vge
